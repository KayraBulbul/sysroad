package main

import (
	"bufio"
	"context"
	"fmt"
	"os"
	"os/exec"
	"os/signal"
	"regexp"
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
	emit(lessonHeading(l, string(mode)))
	r, out, err := a.RunLesson(ctx, l, mode)
	if err != nil {
		return err
	}
	if r.Interactive != nil {
		cmd := r.Interactive
		cmd.Stdin, cmd.Stdout, cmd.Stderr = os.Stdin, os.Stdout, os.Stderr
		err := cmd.Run()
		if _, ok := err.(*exec.ExitError); ok {
			err = nil
		}
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
			if err := syncWithRetry(dir, 20*time.Second); err != nil {
				emit(paint("90", "(not synced yet: "+err.Error()+")") + "\n")
			}
		}
	}
	if !r.OK {
		return errFailed
	}
	return nil
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
	sol, err := l.Solution()
	if err != nil {
		return err
	}
	if len(sol) == 0 {
		fmt.Println("This lesson has no solution files.")
		return nil
	}
	emit(lessonHeading(l, "solution"))
	for _, name := range l.SortedNames(sol) {
		emit("\n" + paint("90", "── "+name+" ──") + "\n")
		fmt.Print(string(sol[name]))
	}
	return nil
}

func runReset(dir string, args []string) error {
	yes := false
	var rest []string
	for _, a := range args {
		if a == "-y" || a == "--yes" {
			yes = true
		} else {
			rest = append(rest, a)
		}
	}
	a, l, err := openLesson(dir, rest)
	if err != nil {
		return err
	}
	if !yes {
		fmt.Printf("Reset %q to the starter code? Your files move to .local/reset-backup/. [y/N] ", l.Title)
		line, _ := bufio.NewReader(os.Stdin).ReadString('\n')
		if t := strings.TrimSpace(strings.ToLower(line)); t != "y" && t != "yes" {
			fmt.Println("Not reset.")
			return nil
		}
	}
	backup, err := a.Engine.Reset(l)
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
	if len(l.Meta.Files) == 0 {
		return fmt.Errorf("%q has no files to edit", l.Title)
	}
	p, err := a.Touch(l)
	if err != nil {
		return err
	}
	cmd := exec.Command("sh", "-c", tui.EditorCommand()+` "$1"`, "sh", l.Meta.Files[0])
	cmd.Dir = p
	cmd.Stdin, cmd.Stdout, cmd.Stderr = os.Stdin, os.Stdout, os.Stderr
	return cmd.Run()
}
