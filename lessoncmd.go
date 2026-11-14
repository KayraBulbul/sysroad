package main

import (
	"bufio"
	"context"
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"regexp"
	"slices"
	"strconv"
	"strings"
	"time"

	"github.com/charmbracelet/x/term"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/gitsync"
	"github.com/KayraBulbul/sysroad/internal/tui"
)

// openLesson opens the data dir and resolves which lesson a command means.
func openLesson(dir string, args []string) (*app.App, *course.Lesson, error) {
	a, err := app.Open(dir)
	if err != nil {
		return nil, nil, err
	}
	id := ""
	if len(args) > 0 {
		id = args[0]
	}
	cwd, _ := os.Getwd()
	l, err := a.ResolveLesson(id, cwd)
	return a, l, err
}

var ansiRE = regexp.MustCompile(`\x1b\[[0-9;]*[A-Za-z]|\x1b\][^\x1b]*\x1b\\`)

// colour reports whether stdout can show ANSI colours. Output inside an
// editor (e.g. nvim's :!) gets plain text.
func colour() bool {
	return os.Getenv("NO_COLOR") == "" && term.IsTerminal(os.Stdout.Fd())
}

func emit(s string) {
	if !colour() {
		s = ansiRE.ReplaceAllString(s, "")
	}
	fmt.Print(s)
}

func paint(code, s string) string { return "\x1b[" + code + "m" + s + "\x1b[0m" }

func width() int {
	if w, _, err := term.GetSize(os.Stdout.Fd()); err == nil && w > 20 {
		return min(w, 100)
	}
	return 80
}

func lessonHeading(l *course.Lesson, what string) string {
	ch := ""
	if l.Chapter != nil {
		ch = paint("90", l.Chapter.Number+" ")
	}
	return ch + paint("1", l.Title) + paint("90", " — "+what) + "\n"
}

func runLesson(dir string, args []string, mode course.Mode) error {
	a, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	if l.IsQuiz() {
		return fmt.Errorf("%s is a quiz — answer it with: sysroad quiz %s", l.ID, l.ID)
	}
	emit(lessonHeading(l, string(mode)))
	r, out, err := a.RunLesson(ctx, l, mode)
	if err != nil {
		return err
	}
	if o := strings.TrimRight(r.Output, "\n"); o != "" {
		emit(o + "\n\n")
	}
	code := "31"
	if r.OK {
		code = "32"
	}
	emit(paint(code+";1", r.Summary) + "\n")
	if out.Completed {
		line := fmt.Sprintf("+%d XP · Lv %d (%d/%d)", out.XPGained, out.Level, out.Into, out.Span)
		if out.LevelUp {
			line += "  ★ level up!"
		}
		emit(paint("33", line) + "\n")
		if l.UnderTheHood() != "" {
			emit(paint("90", "Under the hood is unlocked: sysroad lesson "+l.ID) + "\n")
		}
		if gitsync.IsRepo(dir) {
			if err := gitsync.SyncWait(dir, 20*time.Second); err != nil {
				emit(paint("90", "(not synced yet: "+err.Error()+")") + "\n")
			}
		}
	}
	// An interactive program gets the terminal once the results are out.
	if r.Interactive != nil {
		emit(paint("90", "Starting "+filepath.Base(r.Interactive.Path)+"…") + "\n")
		cmd := r.Interactive
		cmd.Stdin, cmd.Stdout, cmd.Stderr = os.Stdin, os.Stdout, os.Stderr
		if err := cmd.Run(); err != nil {
			if _, ok := err.(*exec.ExitError); !ok {
				return err
			}
		}
	}
	if !r.OK {
		return errFailed
	}
	return nil
}

// runQuiz asks a quiz's unanswered questions one at a time.
func runQuiz(dir string, args []string) error {
	a, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	if !l.IsQuiz() {
		return fmt.Errorf("%s isn't a quiz — try sysroad run", l.ID)
	}
	in := bufio.NewReader(os.Stdin)
	emit(lessonHeading(l, "quiz"))
	for {
		q, err := a.QuizState(l)
		if err != nil {
			return err
		}
		n := q.Next()
		if n < 0 {
			emit(paint("32;1", fmt.Sprintf("✓ %d/%d — quiz complete", q.Solved, q.Total)) + "\n")
			return nil
		}
		it := q.Items[n]
		emit("\n" + paint("1", fmt.Sprintf("Q%d/%d", n+1, q.Total)) + "\n")
		emit(tui.RenderMarkdown(it.Q, width()) + "\n")
		if it.Code != "" {
			emit(tui.RenderMarkdown("```cpp\n"+strings.TrimRight(it.Code, "\n")+"\n```", width()) + "\n")
		}
		for j, opt := range it.Options {
			label := fmt.Sprintf("  %d) ", j+1)
			if slices.Contains(it.Tried, j) {
				label = paint("31", fmt.Sprintf("✗ %d) ", j+1))
			}
			emit(label + tui.RenderMarkdown(opt, width()-6) + "\n")
		}
		for {
			emit(paint("90", fmt.Sprintf("answer 1–%d, q to stop: ", len(it.Options))))
			line, err := in.ReadString('\n')
			line = strings.TrimSpace(line)
			if err != nil || line == "q" {
				fmt.Println()
				return nil
			}
			pick, perr := strconv.Atoi(line)
			if perr != nil || pick < 1 || pick > len(it.Options) {
				continue
			}
			res, err := a.AnswerQuiz(l, n, pick-1)
			if err != nil {
				return err
			}
			if !res.Correct {
				emit(paint("31", "✗ Not that one — try again") + "\n")
				continue
			}
			emit(paint("32", "✓ Right") + "\n")
			if why := res.Quiz.Items[n].Why; why != "" {
				emit(tui.RenderMarkdown(why, width()) + "\n")
			}
			if o := res.Outcome; o.Completed {
				emit(paint("33", fmt.Sprintf("+%d XP · Lv %d (%d/%d)", o.XPGained, o.Level, o.Into, o.Span)) + "\n")
				if gitsync.IsRepo(dir) {
					if err := gitsync.SyncWait(dir, 20*time.Second); err != nil {
						emit(paint("90", "(not synced yet: "+err.Error()+")") + "\n")
					}
				}
			}
			break
		}
	}
}

func runHint(dir string, args []string) error {
	a, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	hints := l.Hints()
	if len(hints) == 0 {
		fmt.Println("No hints for this one.")
		return nil
	}
	before := a.HintsShown(l.ID)
	n, err := a.RevealHint(l)
	if err != nil {
		return err
	}
	emit(lessonHeading(l, fmt.Sprintf("hint %d/%d", n, len(hints))))
	emit(tui.RenderMarkdown(hints[n-1], width()) + "\n")
	if before == n {
		emit(paint("90", "That was the last hint.") + "\n")
	}
	return nil
}

func runSolution(dir string, args []string) error {
	_, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	sol, err := l.PeekSolution()
	if err != nil {
		return err
	}
	if len(sol) == 0 && len(l.Meta.SolutionRemoves) == 0 {
		fmt.Println("This lesson has no solution files.")
		return nil
	}
	if l.Project() != nil {
		emit(lessonHeading(l, "reference: the files this lesson adds or changes"))
	} else {
		emit(lessonHeading(l, "solution"))
	}
	for _, name := range l.SortedNames(sol) {
		emit("\n" + paint("90", "── "+name+" ──") + "\n")
		fmt.Print(string(sol[name]))
	}
	if l.Project() != nil && len(l.Meta.SolutionRemoves) > 0 {
		emit("\n" + paint("90", "── deletes "+strings.Join(l.Meta.SolutionRemoves, ", ")+" ──") + "\n")
	}
	return nil
}

// yesFlag reports whether args hold -y or --yes, which answers the
// question up front, and returns the other args.
func yesFlag(args []string) (bool, []string) {
	var rest []string
	yes := false
	for _, a := range args {
		if a == "-y" || a == "--yes" {
			yes = true
		} else {
			rest = append(rest, a)
		}
	}
	return yes, rest
}

// ask asks a yes/no question on the terminal.
func ask(question string) bool {
	fmt.Print(question + " [y/N] ")
	line, _ := bufio.NewReader(os.Stdin).ReadString('\n')
	t := strings.TrimSpace(strings.ToLower(line))
	return t == "y" || t == "yes"
}

func runReset(dir string, args []string) error {
	yes, rest := yesFlag(args)
	a, l, err := openLesson(dir, rest)
	if err != nil {
		return err
	}
	q := fmt.Sprintf("Reset %q to the starter code? Your files move to .local/reset-backup/.", l.Title)
	if p := l.Project(); p != nil {
		q = fmt.Sprintf("Reset the %s project to how it was at the start of %q? Your files move to .local/reset-backup/.", p.ID, l.Title)
	}
	if !yes && !ask(q) {
		fmt.Println("Not reset.")
		return nil
	}
	backup, err := a.ResetLesson(l)
	if err != nil {
		return err
	}
	if backup != "" {
		fmt.Println("Reset. Your old files are in", backup)
	} else {
		fmt.Println("Created", a.Engine.WorkDir(l))
	}
	return nil
}

func runCatchUp(dir string, args []string) error {
	yes, rest := yesFlag(args)
	a, l, err := openLesson(dir, rest)
	if err != nil {
		return err
	}
	p := l.Project()
	if p == nil {
		return fmt.Errorf("%s isn't part of a project; catching up is for project lessons", l.ID)
	}
	if err := a.CheckOrder(l); err != nil {
		return err
	}
	q := fmt.Sprintf("Replace your %s folder with the course's reference as of the end of %q, and skip that lesson (no XP)? Your files move to .local/reset-backup/.", p.ID, l.Title)
	if !yes && !ask(q) {
		fmt.Println("Not changed.")
		return nil
	}
	backup, err := a.CatchUp(l)
	if err != nil {
		return err
	}
	if backup != "" {
		fmt.Println("Caught up. Your old files are in", backup)
	} else {
		fmt.Println("Caught up.")
	}
	if next := a.ProjectCurrent(p); next != nil {
		fmt.Printf("The %s project is on %q now.\n", p.ID, next.Title)
	}
	return nil
}

func runShowLesson(dir string, args []string) error {
	a, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	emit(tui.LessonText(a, l.ID, width()) + "\n")
	return nil
}

func runPath(dir string, args []string) error {
	a, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	p, err := a.Touch(l)
	if err != nil {
		return err
	}
	fmt.Println(p)
	return nil
}

func runEdit(dir string, args []string) error {
	a, l, err := openLesson(dir, args)
	if err != nil {
		return err
	}
	if !l.HasCode() {
		return fmt.Errorf("%q has no files to edit", l.Title)
	}
	p, err := a.Touch(l)
	if err != nil {
		return err
	}
	file, err := a.EditFile(l)
	if err != nil {
		return err
	}
	cmd := exec.Command("sh", "-c", tui.EditorCommand()+` "$1"`, "sh", file)
	cmd.Dir = p
	cmd.Stdin, cmd.Stdout, cmd.Stderr = os.Stdin, os.Stdout, os.Stderr
	return cmd.Run()
}
