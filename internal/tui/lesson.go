package tui

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"

	tea "github.com/charmbracelet/bubbletea"
	"github.com/charmbracelet/x/ansi"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// lessonView is the boot.dev-style lesson panel: the lesson on top, the
// latest Run/Submit output underneath. The sidebar and the full-screen
// detail view both embed one.
type lessonView struct {
	app *app.App
	id  string

	scroll, outScroll int
	showUTH, showSol  bool
	jump              string // scroll to the first line starting with this, on next render

	running   course.Mode // a Run/Submit in flight, for runningID
	runningID string
	runStart  time.Time
	result    *course.Result
	resMod    time.Time

	confirm string
	onYes   func() tea.Cmd
}

type lessonRunMsg struct {
	id   string
	mode course.Mode
	res  *course.Result
	err  error
}

type lessonEditDoneMsg struct{ err error }

func (v *lessonView) lesson() *course.Lesson { return v.app.Lesson(v.id) }

func (v *lessonView) set(id string) {
	if v.id == id {
		return
	}
	v.id, v.scroll, v.outScroll, v.showUTH, v.showSol, v.confirm = id, 0, 0, false, false, ""
	v.result, v.resMod = nil, time.Time{}
	v.loadResult()
}

// loadResult picks up results written by any process (e.g. `sysroad run`
// from the editor's shell).
func (v *lessonView) loadResult() {
	if v.id == "" {
		return
	}
	if m := v.app.Engine.ResultModTime(v.id); !m.Equal(v.resMod) {
		v.result, v.resMod = v.app.Engine.LastResult(v.id), m
		v.outScroll = 0
	}
}

func (v *lessonView) done() bool { return v.app.Progress.StatusOf(v.id) == store.Done }

// start launches Run or Submit in the background.
func (v *lessonView) start(c *core, mode course.Mode) tea.Cmd {
	l := v.lesson()
	if l == nil {
		return nil
	}
	if v.running != "" {
		if v.runningID != l.ID {
			c.say("Still running %s — one at a time", v.app.Course.Lesson(v.runningID).Title)
		}
		return nil
	}
	switch err := v.app.BeginRun(l); {
	case errors.Is(err, app.ErrManualStep):
		c.say("Nothing to run here — press space when you've done it")
		return nil
	case errors.Is(err, app.ErrUnwritten):
		c.say("This lesson isn't written yet")
		return nil
	case err != nil:
		c.fail(err)
		return nil
	}
	c.changed()
	v.running, v.runningID, v.runStart = mode, l.ID, time.Now()
	engine, dir := v.app.Engine, v.app.Dir
	return func() tea.Msg {
		ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
		defer cancel()
		res, err := app.Execute(ctx, dir, engine, l, mode)
		return lessonRunMsg{id: l.ID, mode: mode, res: res, err: err}
	}
}

// finish records a finished run. It returns a command when the program
// needs the terminal (interactive lessons).
func (v *lessonView) finish(c *core, msg lessonRunMsg) tea.Cmd {
	if msg.id == v.runningID {
		v.running, v.runningID = "", ""
	}
	if msg.err != nil {
		c.fail(msg.err)
		return nil
	}
	l := v.app.Lesson(msg.id)
	if msg.id == v.id {
		v.result, v.resMod, v.outScroll = msg.res, v.app.Engine.ResultModTime(v.id), 0
	}
	out, err := v.app.Record(l, msg.res)
	if err != nil {
		c.fail(err)
		return nil
	}
	if out.Completed {
		c.changed()
		c.say("%s", completedText(l, out))
	}
	if msg.res.Interactive != nil {
		return tea.Exec(&interactiveRun{cmd: msg.res.Interactive}, func(err error) tea.Msg { return lessonEditDoneMsg{err} })
	}
	return nil
}

// interactiveRun hands the terminal to a lesson's program. If it fails,
// it waits for Enter first, or redrawing the sidebar would wipe the error
// the program printed.
type interactiveRun struct {
	cmd *exec.Cmd
	in  io.Reader
	out io.Writer
}

func (r *interactiveRun) SetStdin(in io.Reader)    { r.in, r.cmd.Stdin = in, in }
func (r *interactiveRun) SetStdout(out io.Writer)  { r.out, r.cmd.Stdout = out, out }
func (r *interactiveRun) SetStderr(errw io.Writer) { r.cmd.Stderr = errw }

func (r *interactiveRun) Run() error {
	err := r.cmd.Run()
	var ee *exec.ExitError
	if err != nil && !errors.As(err, &ee) {
		return err
	}
	if err != nil && r.in != nil && r.out != nil {
		fmt.Fprintf(r.out, "\r\n\x1b[90m[%s exited: %s] press Enter to go back\x1b[39m", filepath.Base(r.cmd.Path), ee)
		bufio.NewReader(r.in).ReadString('\n')
	}
	return nil
}

// completedText is the status line for finishing a lesson.
func completedText(l *course.Lesson, out app.Outcome) string {
	s := fmt.Sprintf("✓ Lesson complete  +%d XP", out.XPGained)
	if out.LevelUp {
		s += fmt.Sprintf("  ★ Level %d!", out.Level)
	}
	if l.UnderTheHood() != "" {
		s += "  · u: under the hood"
	}
	return s + "  · ]: next lesson"
}

// completeHow says how a graded lesson gets completed.
func completeHow(l *course.Lesson) string {
	if l.IsQuiz() {
		return "Answer every question to complete a quiz"
	}
	return "Submit (s) completes a lesson"
}

func (v *lessonView) workPath() string {
	l := v.lesson()
	if l == nil {
		return ""
	}
	return v.app.Engine.WorkDir(l)
}

// key handles the lesson keys. handled=false lets the caller try its own.
func (v *lessonView) key(c *core, k string, bodyH int) (tea.Cmd, bool) {
	if v.confirm != "" {
		yes := v.onYes
		v.confirm, v.onYes = "", nil
		if k == "y" || k == "Y" {
			return yes(), true
		}
		return nil, true
	}
	l := v.lesson()
	if l == nil {
		return nil, false
	}
	if l.IsQuiz() {
		if cmd, ok := v.quizKey(c, l, k); ok {
			return cmd, true
		}
	}
	switch k {
	case "r":
		return v.start(c, course.ModeRun), true
	case "s":
		return v.start(c, course.ModeSubmit), true
	case "h":
		if len(l.Hints()) == 0 {
			c.say("No hints for this one")
		} else if n, err := v.app.RevealHint(l); err != nil {
			c.fail(err)
		} else {
			c.say("Hint %d of %d shown", n, len(l.Hints()))
			v.jump = fmt.Sprintf("Hint %d/", n)
		}
		return nil, true
	case "u":
		switch {
		case l.UnderTheHood() == "":
			c.say("No under-the-hood notes for this one")
		case !v.done():
			c.say("Under the hood unlocks when you pass Submit")
		default:
			v.showUTH = !v.showUTH
			if v.showUTH {
				v.jump = "Under the hood"
			}
		}
		return nil, true
	case "v":
		if !l.HasCode() {
			return nil, true
		}
		if v.showSol {
			v.showSol = false
			return nil, true
		}
		v.confirm = "Peek at the solution? (y/N)"
		v.onYes = func() tea.Cmd { v.showSol, v.jump = true, "Solution"; return nil }
		return nil, true
	case "R":
		if !l.HasCode() {
			return nil, true
		}
		v.confirm = "Reset to the starter code? Your files are moved to .local/reset-backup/ (y/N)"
		v.onYes = func() tea.Cmd {
			backup, err := v.app.Engine.Reset(l)
			if err != nil {
				c.fail(err)
			} else if backup != "" {
				c.say("Reset. Your old files are in %s", tildePath(backup))
			}
			c.changed()
			return nil
		}
		return nil, true
	case "e":
		if !l.HasCode() {
			return nil, true
		}
		dir, err := v.app.Touch(l)
		if err != nil {
			c.fail(err)
			return nil, true
		}
		c.changed()
		cmd := exec.Command("sh", "-c", EditorCommand()+` "$1"`, "sh", l.Meta.Files[0])
		cmd.Dir = dir
		return tea.ExecProcess(cmd, func(err error) tea.Msg { return lessonEditDoneMsg{err} }), true
	case "o":
		url := l.URL
		if url == "" && l.Meta.Video != nil {
			url = l.Meta.Video.URL
		}
		if url == "" && len(l.Meta.Deeper) > 0 {
			url = l.Meta.Deeper[0].URL
		}
		c.fail(openURL(url))
		return nil, true
	case "j", "down":
		v.scroll++
	case "k", "up":
		v.scroll--
	case "ctrl+d", "pgdown":
		v.scroll += max(bodyH/2, 1)
	case "ctrl+u", "pgup":
		v.scroll -= max(bodyH/2, 1)
	case "g", "home":
		v.scroll = 0
	case "G", "end":
		v.scroll = 1 << 20
	case "J":
		v.outScroll++
	case "K":
		v.outScroll--
	default:
		return nil, false
	}
	return nil, true
}

// ---- rendering ----

func tildePath(p string) string {
	if home, err := os.UserHomeDir(); err == nil {
		if rest, ok := strings.CutPrefix(p, home+string(filepath.Separator)); ok {
			return "~/" + rest
		}
	}
	return p
}

// chapterDots draws one dot per lesson: done, current, todo, not written.
func chapterDots(a *app.App, ch *course.Chapter, currentID string) string {
	var b strings.Builder
	for _, l := range ch.Lessons {
		switch {
		case l.ID == currentID:
			b.WriteString(sAccent.Render("◉"))
		case a.Progress.StatusOf(l.ID) == store.Done:
			b.WriteString(sGood.Render("●"))
		case !l.Written && l.Detail == "" && l.URL == "":
			b.WriteString(sDim.Render("·"))
		default:
			b.WriteString(sDim.Render("○"))
		}
	}
	return b.String()
}

func levelLine(a *app.App, w int) string {
	xp := a.XP()
	lvl, into, span := course.Level(xp)
	label := sAccent.Render(fmt.Sprintf("Lv %d", lvl))
	count := sDim.Render(fmt.Sprintf(" %d/%d XP", into, span))
	bw := w - ansi.StringWidth(label) - ansi.StringWidth(count) - 1
	frac := 0.0
	if span > 0 {
		frac = float64(into) / float64(span)
	}
	return label + " " + bar(frac, bw) + count
}

func (v *lessonView) bodyLines(w int) []string {
	l := v.lesson()
	if l == nil {
		return nil
	}
	var out []string
	add := func(s string) { out = append(out, strings.Split(s, "\n")...) }
	status := v.app.Progress.StatusOf(l.ID)

	add(wrap(statusIcon(status)+" "+sBold.Render(l.Title), w))
	meta := []string{kindTag(l.Kind)}
	if l.Minutes > 0 {
		meta = append(meta, fmt.Sprintf("~%d min", l.Minutes))
	}
	meta = append(meta, sWarn.Render(fmt.Sprintf("+%d XP", course.XPFor(l))))
	if l.Optional || (l.Chapter != nil && l.Chapter.Phase.Optional) {
		meta = append(meta, "optional")
	}
	if l.Platform != "" {
		meta = append(meta, sWarn.Render(l.Platform))
	}
	add(strings.Join(meta, sDim.Render(" · ")))
	if ch := l.Chapter; ch != nil {
		add(trunc(sDim.Render(ch.Number+" "+ch.Title+" ")+chapterDots(v.app, ch, l.ID), w))
	}
	if l.HasCode() {
		add(trunc(sDim.Render("▸ "+tildePath(v.workPath())+"/"+strings.Join(l.Meta.Files, ", ")), w))
	}
	add("")

	switch {
	case l.Written:
		add(RenderMarkdown(l.Body, w))
	case l.Detail != "":
		add(RenderMarkdown(l.Detail, w))
	default:
		add(sDim.Render(wrap("This lesson isn't written yet. It arrives in a later sysroad update.", w)))
	}
	if l.URL != "" {
		add("")
		add(hyperlink(l.URL, sLink.Render(trunc(l.URL, w))) + sDim.Render("  o: open"))
	}
	if l.IsQuiz() {
		out = append(out, v.quizLines(l, w)...)
	}

	if hints := l.Hints(); len(hints) > 0 {
		shown := min(v.app.HintsShown(l.ID), len(hints))
		for i := 0; i < shown; i++ {
			add("")
			add(sWarn.Render(fmt.Sprintf("Hint %d/%d", i+1, len(hints))))
			add(RenderMarkdown(hints[i], w))
		}
		if shown < len(hints) {
			add("")
			add(sDim.Render(fmt.Sprintf("h: hint (%d left)", len(hints)-shown)))
		}
	}

	if uth := l.UnderTheHood(); uth != "" {
		add("")
		switch {
		case status != store.Done && l.IsQuiz():
			add(sDim.Render("🔒 Under the hood unlocks when you finish the quiz"))
		case status != store.Done:
			add(sDim.Render("🔒 Under the hood unlocks when you pass Submit"))
		case v.showUTH:
			add(sHeading.Render("Under the hood"))
			add(RenderMarkdown(uth, w))
		default:
			add(sGood.Render("✦ Under the hood is unlocked") + sDim.Render(" — u to read"))
		}
	}

	if v.showSol {
		sol, _ := l.Solution()
		add("")
		add(sHeading.Render("Solution"))
		for _, name := range l.SortedNames(sol) {
			add(sDim.Render(name))
			add(strings.Join(codeBlock(strings.TrimRight(string(sol[name]), "\n"), langOf(name), w), "\n"))
		}
	}

	var links []string
	if vid := l.Meta.Video; vid != nil {
		links = append(links, "▶ "+hyperlink(vid.URL, sLink.Render(vid.Title)))
	}
	for _, d := range l.Meta.Deeper {
		links = append(links, "• "+hyperlink(d.URL, sLink.Render(d.Title)))
	}
	if len(links) > 0 {
		add("")
		add(sDim.Render("Go deeper (optional)"))
		for _, s := range links {
			add(trunc(s, w))
		}
	}
	if note := v.app.Progress.Notes[l.ID]; note != "" {
		add("")
		add(sHeading.Render("✎ notes"))
		add(wrap(note, w))
	}
	return out
}

// applyJump scrolls to a pending jump target, if it's in body.
func (v *lessonView) applyJump(body []string) {
	if v.jump == "" {
		return
	}
	for i, l := range body {
		if strings.HasPrefix(ansi.Strip(l), v.jump) {
			v.scroll = i
			break
		}
	}
	v.jump = ""
}

// visibleBody scrolls body to fit h lines, marking the last one when
// there's more below.
func (v *lessonView) visibleBody(body []string, h int) []string {
	v.applyJump(body)
	maxScroll := max(len(body)-h, 0)
	v.scroll = min(max(v.scroll, 0), maxScroll)
	lines := append([]string{}, body[v.scroll:min(v.scroll+h, len(body))]...)
	if len(lines) == h && maxScroll > 0 && v.scroll < maxScroll {
		lines[h-1] = sDim.Render(fmt.Sprintf("↓ more (j/k scroll · %d%%)", 100*v.scroll/maxScroll))
	}
	return lines
}

func langOf(name string) string {
	switch filepath.Ext(name) {
	case ".cpp", ".cc", ".hpp", ".h":
		return "cpp"
	}
	return ""
}

func (v *lessonView) outputLines(w int) []string {
	if v.running != "" && v.runningID == v.id {
		verb := "Running"
		if v.running == course.ModeSubmit {
			verb = "Submitting"
		}
		return []string{sWarn.Render(fmt.Sprintf("⟳ %s… %.0fs", verb, time.Since(v.runStart).Seconds()))}
	}
	if l := v.lesson(); l != nil && l.IsQuiz() {
		return nil // the questions are the whole lesson
	}
	r := v.result
	if r == nil {
		l := v.lesson()
		if l != nil && l.Graded() {
			return []string{sDim.Render("No runs yet — r to run, s to submit")}
		}
		return nil
	}
	st := sErr
	if r.OK {
		st = sGood
	}
	head := st.Render(r.Summary) + sDim.Render(fmt.Sprintf("  %s · %s", r.Mode, ago(r.At)))
	out := []string{trunc(head, w)}
	text := strings.TrimRight(r.Output, "\n")
	if text != "" {
		for _, l := range strings.Split(text, "\n") {
			out = append(out, strings.Split(ansi.Wrap(strings.ReplaceAll(l, "\t", "    "), w, ""), "\n")...)
		}
	}
	return out
}

// screen lays out the lesson (scrollable) above its output pane.
func (v *lessonView) screen(w, h int) []string {
	if h < 4 {
		return nil
	}
	outAll := v.outputLines(w)
	outH := 0
	if len(outAll) > 0 {
		outH = min(len(outAll), max(h*45/100, 3))
		if v.result != nil && !v.result.OK {
			outH = min(len(outAll), max(h*55/100, 3))
		}
	}
	bodyH := h - outH
	if outH > 0 {
		bodyH-- // separator
	}
	lines := v.visibleBody(v.bodyLines(w), bodyH)
	for len(lines) < bodyH {
		lines = append(lines, "")
	}
	if outH > 0 {
		v.outScroll = min(max(v.outScroll, 0), max(len(outAll)-outH, 0))
		label := "─ Output "
		if len(outAll) > outH {
			label = fmt.Sprintf("─ Output (J/K %d/%d) ", v.outScroll+outH, len(outAll))
		}
		lines = append(lines, sDim.Render(label+strings.Repeat("─", max(w-ansi.StringWidth(label), 0))))
		// The summary line stays put; the rest scrolls.
		lines = append(lines, outAll[0])
		rest := outAll[1:]
		start := min(v.outScroll, len(rest))
		end := min(start+outH-1, len(rest))
		lines = append(lines, rest[start:end]...)
	}
	return lines
}

func (v *lessonView) hints(c *core) string {
	if v.confirm != "" {
		return " " + sWarn.Render(v.confirm)
	}
	l := v.lesson()
	if l == nil {
		return ""
	}
	if !l.Graded() {
		return keyHints([][2]string{{"spc", "done"}, {"o", "open"}, {"[/]", "prev/next"}, {"n", "note"}, {"q", ""}})
	}
	if l.IsQuiz() {
		keys := [][2]string{{"1-9", "answer"}, {"R", "retake"}}
		if v.done() && l.UnderTheHood() != "" {
			keys = append(keys, [2]string{"u", "hood"})
		}
		return keyHints(append(keys, [2]string{"[/]", "prev/next"}, [2]string{"q", ""}))
	}
	keys := [][2]string{{"r", "run"}, {"s", "submit"}, {"h", "hint"}}
	if v.done() && l.UnderTheHood() != "" {
		keys = append(keys, [2]string{"u", "hood"})
	}
	if l.HasCode() {
		keys = append(keys, [2]string{"e", "edit"}, [2]string{"v", "solution"})
	}
	keys = append(keys, [2]string{"[/]", "prev/next"}, [2]string{"q", ""})
	return keyHints(keys)
}

// LessonText renders a lesson for `sysroad lesson`: the same text the
// panel shows, with Under the hood expanded once it's unlocked.
func LessonText(a *app.App, id string, w int) string {
	v := lessonView{app: a, id: id}
	v.showUTH = v.done()
	return strings.Join(v.bodyLines(w), "\n")
}

// split is the wide layout: the lesson on the left, the output on the right.
func (v *lessonView) split(w, h int) []string {
	lw := min(w*52/100, 96)
	rw := w - lw - 3
	left := v.visibleBody(v.bodyLines(lw), h)

	out := v.outputLines(rw)
	right := []string{sDim.Render("Output")}
	if len(out) > 0 {
		right = append(right, out[0])
		rest := out[1:]
		room := h - 2
		v.outScroll = min(max(v.outScroll, 0), max(len(rest)-room, 0))
		end := min(v.outScroll+room, len(rest))
		right = append(right, rest[v.outScroll:end]...)
		if len(rest) > room {
			right[0] = sDim.Render(fmt.Sprintf("Output (J/K %d/%d)", end, len(rest)))
		}
	}
	rows := make([]string, h)
	sep := sDim.Render(" │ ")
	for i := range rows {
		l, r := "", ""
		if i < len(left) {
			l = left[i]
		}
		if i < len(right) {
			r = right[i]
		}
		rows[i] = pad(trunc(l, lw), lw) + sep + trunc(r, rw)
	}
	return rows
}
