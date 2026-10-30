package tui

import (
	"fmt"
	"strings"
	"time"

	"github.com/charmbracelet/bubbles/textinput"
	"github.com/charmbracelet/bubbles/viewport"
	tea "github.com/charmbracelet/bubbletea"
	"github.com/charmbracelet/lipgloss"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/roadmap"
	"github.com/KayraBulbul/sysroad/internal/store"
)

type pane int

const (
	paneSections pane = iota
	paneSteps
)

type mode int

const (
	modeNormal mode = iota
	modeInput
	modeConfirm
	modeHelp
	modeDetail
)

type inputPurpose int

const (
	inAddStep inputPurpose = iota
	inAddSection
	inRename
	inLogTime
)

type Full struct {
	core
	focus     pane
	secCur    int
	stepCur   int
	mode      mode
	input     textinput.Model
	purpose   inputPurpose
	confirm   string
	onConfirm func()
	vp        viewport.Model
}

func NewFull(a *app.App) *Full {
	ti := textinput.New()
	ti.CharLimit = 200
	m := &Full{core: newCore(a), input: ti}
	m.jumpToCurrent()
	return m
}

func (m *Full) Init() tea.Cmd { return m.core.init() }

func (m *Full) sec() *roadmap.Section {
	secs := m.app.Roadmap.Sections
	if len(secs) == 0 {
		return nil
	}
	m.secCur = min(max(m.secCur, 0), len(secs)-1)
	return secs[m.secCur]
}

func (m *Full) step() *roadmap.Step {
	s := m.sec()
	if s == nil || len(s.Steps) == 0 {
		return nil
	}
	m.stepCur = min(max(m.stepCur, 0), len(s.Steps)-1)
	return s.Steps[m.stepCur]
}

// selectedID is the step under the cursor in the steps pane, else the section.
func (m *Full) selectedID() string {
	if m.focus == paneSteps || m.mode == modeDetail {
		if st := m.step(); st != nil {
			return st.ID
		}
	}
	if s := m.sec(); s != nil {
		return s.ID
	}
	return ""
}

func (m *Full) jumpToCurrent() {
	cur, ok := m.app.Current()
	if !ok {
		return
	}
	for i, s := range m.app.Roadmap.Sections {
		for j, st := range s.Steps {
			if st.ID == cur.Step.ID {
				m.secCur, m.stepCur, m.focus = i, j, paneSteps
				return
			}
		}
	}
}

func (m *Full) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	if cmd, handled := m.core.update(msg); handled {
		if m.mode == modeDetail {
			m.refreshDetail(false)
		}
		return m, cmd
	}
	key, ok := msg.(tea.KeyMsg)
	if !ok {
		return m, nil
	}
	if key.String() == "ctrl+c" {
		return m, tea.Quit
	}
	switch m.mode {
	case modeInput:
		return m, m.updateInput(key)
	case modeConfirm:
		if key.String() == "y" || key.String() == "Y" {
			m.onConfirm()
		}
		m.mode = modeNormal
		return m, nil
	case modeHelp:
		m.mode = modeNormal
		return m, nil
	case modeDetail:
		return m, m.updateDetail(key)
	}
	return m, m.updateNormal(key)
}

func (m *Full) updateNormal(key tea.KeyMsg) tea.Cmd {
	sec := m.sec()
	switch key.String() {
	case "q":
		return tea.Quit
	case "?":
		m.mode = modeHelp
	case "j", "down":
		m.moveCursor(1)
	case "k", "up":
		m.moveCursor(-1)
	case "ctrl+d", "pgdown":
		m.moveCursor(10)
	case "ctrl+u", "pgup":
		m.moveCursor(-10)
	case "g", "home":
		m.moveCursor(-1 << 20)
	case "G", "end":
		m.moveCursor(1 << 20)
	case "h", "left":
		m.focus = paneSections
	case "l", "right":
		m.focus = paneSteps
	case "tab":
		m.focus = 1 - m.focus
	case "enter":
		if m.focus == paneSections {
			m.focus, m.stepCur = paneSteps, m.firstOpenStep(sec)
		} else if m.step() != nil {
			m.openDetail()
		}
	case "c":
		m.jumpToCurrent()
	case " ":
		if m.focus == paneSteps && m.step() != nil {
			m.toggleStatus(m.step().ID, store.Done)
			m.advanceAfterDone()
		}
	case "p":
		if m.focus == paneSteps && m.step() != nil {
			m.toggleStatus(m.step().ID, store.Doing)
		}
	case "x":
		if m.focus == paneSteps && m.step() != nil {
			m.toggleStatus(m.step().ID, store.Skipped)
		}
	case "t":
		if id := m.selectedID(); id != "" {
			m.toggleTimer(id)
		}
	case "L":
		if m.selectedID() != "" {
			m.prompt(inLogTime, "Log time on "+trunc(m.app.Roadmap.TitleOf(m.selectedID()), 40)+" (45m, 1h30m): ", "")
		}
	case "n":
		if id := m.selectedID(); id != "" {
			return m.editNote(id)
		}
	case "N":
		if sec != nil {
			return m.editNote(sec.ID)
		}
	case "e":
		if m.focus == paneSteps && m.step() != nil {
			return m.editStep(m.step().ID)
		} else if sec != nil {
			return m.editSection(sec.ID)
		}
	case "a":
		if m.focus == paneSteps && sec != nil {
			m.prompt(inAddStep, "New step in "+trunc(sec.Title, 30)+": ", "")
		} else {
			m.prompt(inAddSection, "New section title: ", "")
		}
	case "r":
		if id := m.selectedID(); id != "" {
			m.prompt(inRename, "Rename: ", m.app.Roadmap.TitleOf(id))
		}
	case "D":
		m.askDelete()
	case "J", "shift+down":
		m.moveItem(1)
	case "K", "shift+up":
		m.moveItem(-1)
	case "o":
		url := ""
		if m.focus == paneSteps && m.step() != nil {
			url = m.step().URL
		}
		if url == "" && sec != nil {
			url = sectionURL(sec)
		}
		m.fail(openURL(url))
	case "s":
		if !m.hasRepo {
			m.say("Data dir isn't a git repo — see `sysroad init --help`")
		}
		return m.syncNow()
	}
	return nil
}

func (m *Full) firstOpenStep(s *roadmap.Section) int {
	if s == nil {
		return 0
	}
	for i, st := range s.Steps {
		if st := m.app.Progress.StatusOf(st.ID); st == store.Todo || st == store.Doing {
			return i
		}
	}
	return 0
}

// advanceAfterDone moves the cursor to the next step once one is completed.
func (m *Full) advanceAfterDone() {
	if st := m.step(); st != nil && m.app.Progress.StatusOf(st.ID) == store.Done {
		if m.stepCur < len(m.sec().Steps)-1 {
			m.stepCur++
		}
	}
}

func (m *Full) moveCursor(d int) {
	if m.focus == paneSections {
		m.secCur = min(max(m.secCur+d, 0), max(len(m.app.Roadmap.Sections)-1, 0))
		m.stepCur = 0
		return
	}
	if s := m.sec(); s != nil {
		m.stepCur = min(max(m.stepCur+d, 0), max(len(s.Steps)-1, 0))
	}
}

func (m *Full) moveItem(d int) {
	r := m.app.Roadmap
	if m.focus == paneSections {
		m.secCur = roadmap.Move(r.Sections, m.secCur, d)
	} else if s := m.sec(); s != nil {
		m.stepCur = roadmap.Move(s.Steps, m.stepCur, d)
	}
	m.check(m.app.SaveRoadmap())
}

func (m *Full) askDelete() {
	r := m.app.Roadmap
	sec := m.sec()
	if sec == nil {
		return
	}
	if m.focus == paneSteps {
		st := m.step()
		if st == nil {
			return
		}
		m.confirm = fmt.Sprintf("Delete step %q? (y/N)", trunc(st.Title, 50))
		m.onConfirm = func() {
			sec.Steps = roadmap.Remove(sec.Steps, m.stepCur)
			m.check(m.app.SaveRoadmap())
		}
	} else {
		m.confirm = fmt.Sprintf("Delete section %q and its %d steps? (y/N)", trunc(sec.Title, 40), len(sec.Steps))
		m.onConfirm = func() {
			r.Sections = roadmap.Remove(r.Sections, m.secCur)
			m.check(m.app.SaveRoadmap())
		}
	}
	m.mode = modeConfirm
}

func (m *Full) prompt(p inputPurpose, label, value string) {
	m.purpose, m.mode = p, modeInput
	m.input.Prompt = label
	m.input.SetValue(value)
	m.input.CursorEnd()
	m.input.Focus()
}

func (m *Full) updateInput(key tea.KeyMsg) tea.Cmd {
	switch key.String() {
	case "esc":
		m.mode = modeNormal
		return nil
	case "enter":
		m.mode = modeNormal
		m.submitInput(strings.TrimSpace(m.input.Value()))
		return nil
	}
	var cmd tea.Cmd
	m.input, cmd = m.input.Update(key)
	return cmd
}

func (m *Full) submitInput(v string) {
	if v == "" {
		return
	}
	r := m.app.Roadmap
	switch m.purpose {
	case inAddStep:
		sec := m.sec()
		st := &roadmap.Step{ID: roadmap.NewID(sec.ID, v), Title: v, Kind: roadmap.KindTask}
		at := m.stepCur + 1
		if len(sec.Steps) == 0 {
			at = 0
		}
		sec.Steps = roadmap.Insert(sec.Steps, at, st)
		m.stepCur, m.focus = at, paneSteps
		m.check(m.app.SaveRoadmap())
		m.say("Added — press e to set kind, hours, link and details")
	case inAddSection:
		s := &roadmap.Section{ID: roadmap.NewID("", v), Title: v}
		at := m.secCur + 1
		r.Sections = roadmap.Insert(r.Sections, at, s)
		m.secCur, m.stepCur, m.focus = at, 0, paneSections
		m.check(m.app.SaveRoadmap())
	case inRename:
		id := m.selectedID()
		if s := r.Section(id); s != nil {
			s.Title = v
		} else if _, st := r.Find(id); st != nil {
			st.Title = v
		}
		m.check(m.app.SaveRoadmap())
	case inLogTime:
		mins, err := store.ParseMinutes(v)
		if err != nil {
			m.fail(err)
			return
		}
		m.check(m.app.LogTime(m.selectedID(), mins))
		m.say("Logged %s", store.FormatMinutes(mins))
	}
}

// ---- detail view ----

func (m *Full) openDetail() {
	m.mode = modeDetail
	m.vp = viewport.New(m.w, m.h-2)
	m.refreshDetail(true)
}

func (m *Full) refreshDetail(top bool) {
	m.vp.Width, m.vp.Height = m.w, max(m.h-2, 1)
	st := m.step()
	if st == nil {
		m.mode = modeNormal
		return
	}
	body := m.stepDetail(m.sec(), st, min(m.w-6, 100), true)
	m.vp.SetContent("\n  " + strings.ReplaceAll(body, "\n", "\n  "))
	if top {
		m.vp.GotoTop()
	}
}

func (m *Full) updateDetail(key tea.KeyMsg) tea.Cmd {
	switch key.String() {
	case "esc", "q", "enter", "backspace":
		m.mode = modeNormal
		return nil
	case "J", "]":
		m.moveCursor(1)
		m.refreshDetail(true)
		return nil
	case "K", "[":
		m.moveCursor(-1)
		m.refreshDetail(true)
		return nil
	case " ", "p", "x", "t", "L", "n", "e", "o":
		prev := m.focus
		m.focus = paneSteps
		cmd := m.updateNormal(key)
		m.focus = prev
		if m.mode == modeDetail {
			m.refreshDetail(false)
		}
		return cmd
	}
	var cmd tea.Cmd
	m.vp, cmd = m.vp.Update(key)
	return cmd
}

// ---- rendering ----

func (m *Full) View() string {
	if m.w == 0 {
		return ""
	}
	header := m.header()
	footer := m.footer()
	if m.mode == modeDetail {
		return lipgloss.JoinVertical(lipgloss.Left, header, m.vp.View(), footer)
	}
	if m.mode == modeHelp {
		return lipgloss.JoinVertical(lipgloss.Left, header, clip(helpText(), m.h-2), footer)
	}
	bodyH := m.h - 2
	leftW := min(max(m.w*32/100, 28), 46)
	rightW := m.w - leftW
	stepsH := max(bodyH*55/100, 6)
	detailH := bodyH - stepsH

	left := m.box(m.renderSections(leftW-2, bodyH-2), leftW, bodyH, m.focus == paneSections, "Sections")
	sec := m.sec()
	title := "Steps"
	if sec != nil {
		title = sec.Title
	}
	steps := m.box(m.renderSteps(rightW-2, stepsH-2), rightW, stepsH, m.focus == paneSteps, title)
	var detail string
	if m.focus == paneSteps && m.step() != nil {
		detail = m.stepDetail(sec, m.step(), rightW-4, false)
	} else if sec != nil {
		detail = m.sectionDetail(sec, rightW-4)
	}
	detailBox := m.box(clip(" "+strings.ReplaceAll(detail, "\n", "\n "), detailH-2), rightW, detailH, false, "")
	body := lipgloss.JoinHorizontal(lipgloss.Top, left, lipgloss.JoinVertical(lipgloss.Left, steps, detailBox))
	return lipgloss.JoinVertical(lipgloss.Left, header, body, footer)
}

func (m *Full) box(content string, w, h int, focused bool, title string) string {
	border := cDim
	if focused {
		border = cBlue
	}
	st := lipgloss.NewStyle().Border(lipgloss.RoundedBorder()).BorderForeground(border).
		Width(w - 2).Height(h - 2).MaxHeight(h)
	out := st.Render(clip(content, h-2))
	if title != "" {
		// Draw the title into the top border.
		lines := strings.SplitN(out, "\n", 2)
		t := " " + trunc(title, w-6) + " "
		ts := sDim
		if focused {
			ts = sAccent
		}
		bs := lipgloss.NewStyle().Foreground(border)
		top := bs.Render("╭─") + ts.Render(t) + bs.Render(strings.Repeat("─", max(w-3-lipgloss.Width(t), 0))+"╮")
		if len(lines) == 2 {
			out = top + "\n" + lines[1]
		}
	}
	return out
}

func (m *Full) header() string {
	ov := m.app.Overall()
	now := time.Now()
	// In priority order; the tail is dropped when the terminal is narrow.
	parts := []string{
		m.streakText(),
		"today " + store.FormatMinutes(m.app.TodayMinutes(now)),
		m.timerText(),
		fmt.Sprintf("%d/%d (%.0f%%)", ov.Done, ov.Total, ov.Frac()*100),
		m.syncStatus(),
	}
	if eta := m.app.ETA(now); eta != "" {
		parts = append(parts, sDim.Render(eta+" left"))
	}
	sep := sDim.Render(" · ")
	var right string
	for n := len(parts); n > 0; n-- {
		right = strings.Join(nonEmpty(parts[:n]), sep) + " "
		if lipgloss.Width(right)+9 <= m.w {
			break
		}
	}
	left := sAccent.Render(" sysroad") + sDim.Render(" · "+m.app.Roadmap.Title)
	return spread(left, right, m.w)
}

func nonEmpty(ss []string) []string {
	var out []string
	for _, s := range ss {
		if s != "" {
			out = append(out, s)
		}
	}
	return out
}

func (m *Full) footer() string {
	var s string
	switch {
	case m.mode == modeInput:
		s = " " + m.input.View()
	case m.mode == modeConfirm:
		s = " " + sWarn.Render(m.confirm)
	case m.flash != "":
		st := sGood
		if m.flashErr {
			st = sErr
		}
		s = " " + st.Render(m.flash)
	case m.syncErr != nil && m.flash == "":
		s = " " + sErr.Render("sync: "+m.syncErr.Error())
	case m.mode == modeDetail:
		s = keyHints([][2]string{{"esc", "back"}, {"J/K", "next/prev"}, {"space", "done"}, {"t", "timer"}, {"n", "note"}, {"e", "edit"}, {"o", "open"}})
	case m.mode == modeHelp:
		s = sDim.Render(" any key to close")
	default:
		s = keyHints([][2]string{{"space", "done"}, {"p", "doing"}, {"t", "timer"}, {"n", "note"}, {"enter", "open"}, {"e", "edit"}, {"a", "add"}, {"c", "current"}, {"?", "help"}, {"q", "quit"}})
	}
	return trunc(s, m.w)
}

func keyHints(keys [][2]string) string {
	var b strings.Builder
	for _, k := range keys {
		b.WriteString(" " + sKey.Render(k[0]) + sDim.Render(" "+k[1]+" "))
	}
	return b.String()
}

func (m *Full) renderSections(w, h int) string {
	secs := m.app.Roadmap.Sections
	start, end := window(len(secs), m.secCur, h/2)
	var lines []string
	for i := start; i < end; i++ {
		s := secs[i]
		st := m.app.SectionStats(s)
		title := trunc(s.Title, w-2)
		if s.Optional {
			title = trunc(s.Title, w-6) + sDim.Render(" opt")
		}
		mark := "  "
		if i == m.secCur {
			mark = sAccent.Render("▸ ")
			if m.focus == paneSections {
				title = sBold.Render(title)
			}
		}
		if st.Total > 0 && st.Done == st.Total {
			title = sGood.Render(title)
		}
		count := fmt.Sprintf(" %d/%d", st.Done, st.Total)
		lines = append(lines, mark+title)
		lines = append(lines, "  "+bar(st.Frac(), w-3-len(count))+sDim.Render(count))
	}
	return strings.Join(lines, "\n")
}

func (m *Full) renderSteps(w, h int) string {
	sec := m.sec()
	if sec == nil {
		return sDim.Render("No sections — press a to add one")
	}
	if len(sec.Steps) == 0 {
		return sDim.Render("No steps — press a to add one")
	}
	cur, _ := m.app.Current()
	start, end := window(len(sec.Steps), m.stepCur, h)
	var lines []string
	for i := start; i < end; i++ {
		st := sec.Steps[i]
		status := m.app.Progress.StatusOf(st.ID)
		var right []string
		if mins := m.app.Progress.Minutes[st.ID]; mins > 0 {
			right = append(right, sWarn.Render(store.FormatMinutes(mins)))
		}
		if st.Hours > 0 {
			right = append(right, sDim.Render(fmt.Sprintf("~%gh", st.Hours)))
		}
		if st.Platform != "" {
			right = append(right, sDim.Render("["+st.Platform+"]"))
		}
		if st.Optional {
			right = append(right, sDim.Render("opt"))
		}
		if m.app.Timer != nil && m.app.Timer.ID == st.ID {
			right = append(right, sWarn.Render("◷"))
		}
		if m.app.Progress.Notes[st.ID] != "" {
			right = append(right, sDim.Render("✎"))
		}
		title := st.Title
		switch status {
		case store.Done:
			title = sDim.Render(title)
		case store.Skipped:
			title = sDim.Strikethrough(true).Render(title)
		}
		if cur.Step != nil && cur.Step.ID == st.ID {
			title = sAccent.Render("→ ") + title
		}
		left := fmt.Sprintf("%2d. %s %s %s", i+1, statusIcon(status), kindTag(st.Kind), title)
		line := spread(left, strings.Join(right, " "), w)
		if i == m.stepCur && m.focus == paneSteps {
			line = sCursor.Render(pad(stripStyle(line), w))
		}
		lines = append(lines, line)
	}
	return strings.Join(lines, "\n")
}

func (m *Full) stepDetail(sec *roadmap.Section, st *roadmap.Step, w int, full bool) string {
	p := m.app.Progress
	var b strings.Builder
	b.WriteString(wrap(sBold.Render(st.Title), w) + "\n")
	meta := []string{kindTag(st.Kind), statusIcon(p.StatusOf(st.ID)) + " " + string(p.StatusOf(st.ID))}
	if st.Hours > 0 {
		meta = append(meta, fmt.Sprintf("est %gh", st.Hours))
	}
	meta = append(meta, "logged "+store.FormatMinutes(p.Minutes[st.ID]))
	if st.Platform != "" {
		meta = append(meta, sWarn.Render(st.Platform))
	}
	if st.Optional {
		meta = append(meta, "optional")
	}
	b.WriteString(sDim.Render(strings.Join(meta, sDim.Render(" · "))) + "\n")
	if st.URL != "" {
		b.WriteString(sDim.Render("↗ ") + lipgloss.NewStyle().Foreground(cBlue).Underline(true).Render(trunc(st.URL, w-2)) + "\n")
	}
	if st.Detail != "" {
		b.WriteString("\n" + wrap(reflow(st.Detail), w) + "\n")
	}
	if st.DoneWhen != "" {
		b.WriteString("\n" + sGood.Bold(true).Render("Done when") + "\n" + wrap(reflow(st.DoneWhen), w) + "\n")
	}
	if note := p.Notes[st.ID]; note != "" {
		b.WriteString("\n" + sHeading.Render("Notes") + "\n" + wrap(note, w) + "\n")
	} else {
		b.WriteString("\n" + sDim.Render("No notes yet — press n to write some.") + "\n")
	}
	if full {
		if note := p.Notes[sec.ID]; note != "" {
			b.WriteString("\n" + sHeading.Render("Section notes") + sDim.Render(" ("+sec.Title+")") + "\n" + wrap(note, w) + "\n")
		}
		if log := m.timeLog(st.ID, 10); log != "" {
			b.WriteString("\n" + sHeading.Render("Time log") + "\n" + log)
		}
	}
	return b.String()
}

func (m *Full) timeLog(id string, n int) string {
	var lines []string
	ev := m.app.Progress.Events
	for i := len(ev) - 1; i >= 0 && len(lines) < n; i-- {
		if e := ev[i]; e.Type == store.EvTime && e.ID == id {
			lines = append(lines, fmt.Sprintf("  %s  %6s  %s", e.T.Local().Format("Mon 02 Jan 15:04"), store.FormatMinutes(e.Minutes), sDim.Render(e.Host)))
		}
	}
	return strings.Join(lines, "\n")
}

func (m *Full) sectionDetail(sec *roadmap.Section, w int) string {
	st := m.app.SectionStats(sec)
	var b strings.Builder
	b.WriteString(sBold.Render(sec.Title) + "\n")
	b.WriteString(sDim.Render(fmt.Sprintf("%d/%d steps · ~%.0fh left of ~%.0fh · logged %s",
		st.Done, st.Total, st.HoursLeft, st.HoursEstimated, store.FormatMinutes(st.Minutes))) + "\n")
	if sec.Summary != "" {
		b.WriteString("\n" + wrap(reflow(sec.Summary), w) + "\n")
	}
	if len(sec.Resources) > 0 {
		b.WriteString("\n" + sHeading.Render("Resources") + "\n")
		for _, r := range sec.Resources {
			line := "• " + r.Title
			if r.Paid {
				line += sWarn.Render(" $")
			}
			if r.Note != "" {
				line += sDim.Render(" — " + r.Note)
			}
			b.WriteString(trunc(line, w) + "\n")
			if r.URL != "" {
				b.WriteString("  " + sDim.Render(trunc(r.URL, w-2)) + "\n")
			}
		}
	}
	if note := m.app.Progress.Notes[sec.ID]; note != "" {
		b.WriteString("\n" + sHeading.Render("Notes") + "\n" + wrap(note, w) + "\n")
	} else {
		b.WriteString("\n" + sDim.Render("No section notes — press n to write some.") + "\n")
	}
	return b.String()
}

func helpText() string {
	rows := [][2]string{
		{"", "Navigation"},
		{"j/k ↑/↓", "move"}, {"h/l tab", "switch pane"}, {"enter", "open section / step detail"},
		{"g/G", "top / bottom"}, {"c", "jump to current step"},
		{"", "Progress"},
		{"space", "toggle done"}, {"p", "toggle in progress"}, {"x", "toggle skipped"},
		{"t", "start/stop timer on the selected item"}, {"L", "log time manually"},
		{"n", "edit note (step, or section in the left pane)"}, {"N", "edit the section's note"},
		{"", "Editing the roadmap"},
		{"a", "add step (right pane) / section (left pane)"}, {"r", "rename"},
		{"e", "edit in $EDITOR as YAML (kind, hours, link, details…)"},
		{"J/K", "move item down / up"}, {"D", "delete"},
		{"", "Other"},
		{"o", "open link in browser"}, {"s", "sync now"}, {"q", "quit (syncs first)"},
	}
	var b strings.Builder
	b.WriteString("\n")
	for _, r := range rows {
		if r[0] == "" {
			b.WriteString("\n  " + sHeading.Render(r[1]) + "\n")
			continue
		}
		b.WriteString("  " + sKey.Render(pad(r[0], 10)) + " " + r[1] + "\n")
	}
	return b.String()
}
