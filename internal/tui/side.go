package tui

import (
	"fmt"
	"strings"
	"time"

	tea "github.com/charmbracelet/bubbletea"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// Side is the narrow view meant for a terminal split: the current step in
// detail, then what's in progress, up next and recently done.
type Side struct {
	core
	sel   int // index into selectable()
	nextN int // upcoming steps that fit on screen, set by View
}

func NewSide(a *app.App) *Side { return &Side{core: newCore(a), nextN: 5} }

func (m *Side) Init() tea.Cmd { return m.core.init() }

// selectable is the list j/k moves through: current, other in-progress, upcoming.
func (m *Side) selectable() []app.Ref {
	cur, ok := m.app.Current()
	if !ok {
		return nil
	}
	refs := []app.Ref{cur}
	refs = append(refs, m.app.Doing(cur.Step.ID)...)
	return append(refs, m.app.Upcoming(m.nextN)...)
}

func (m *Side) selected() (app.Ref, bool) {
	refs := m.selectable()
	if len(refs) == 0 {
		return app.Ref{}, false
	}
	m.sel = min(max(m.sel, 0), len(refs)-1)
	return refs[m.sel], true
}

func (m *Side) Update(msg tea.Msg) (tea.Model, tea.Cmd) {
	if cmd, handled := m.core.update(msg); handled {
		return m, cmd
	}
	key, ok := msg.(tea.KeyMsg)
	if !ok {
		return m, nil
	}
	ref, hasSel := m.selected()
	switch key.String() {
	case "q", "ctrl+c":
		return m, tea.Quit
	case "j", "down":
		m.sel++
	case "k", "up":
		m.sel--
	case "esc", "c":
		m.sel = 0
	case " ":
		if hasSel {
			m.toggleStatus(ref.Step.ID, store.Done)
			m.sel = 0
		}
	case "p":
		if hasSel {
			m.toggleStatus(ref.Step.ID, store.Doing)
		}
	case "t":
		if hasSel {
			m.toggleTimer(ref.Step.ID)
		}
	case "n":
		if hasSel {
			return m, m.editNote(ref.Step.ID)
		}
	case "o":
		if hasSel {
			url := ref.Step.URL
			if url == "" {
				url = sectionURL(ref.Sec)
			}
			m.fail(openURL(url))
		}
	case "s":
		return m, m.syncNow()
	}
	return m, nil
}

func (m *Side) View() string {
	if m.w == 0 {
		return ""
	}
	w := m.w - 2 // one column of margin each side
	var top, bottom []string
	out := &top
	line := func(s string) { *out = append(*out, " "+trunc(s, w)) }
	block := func(s string, indent int) {
		for _, l := range strings.Split(wrap(s, w-indent), "\n") {
			line(strings.Repeat(" ", indent) + l)
		}
	}
	heading := func(s string) { *out = append(*out, ""); line(sHeading.Render(s)) }
	item := func(i int, r app.Ref) {
		s := statusIcon(m.app.Progress.StatusOf(r.Step.ID)) + " " + r.Step.Title
		if i == m.sel {
			s = sCursor.Render(pad(stripStyle(s), w))
		}
		line(s)
	}

	now := time.Now()
	ov := m.app.Overall()
	line(spread(sAccent.Render("sysroad"), m.streakText()+sDim.Render(" · ")+store.FormatMinutes(m.app.TodayMinutes(now)), w))
	count := fmt.Sprintf(" %d/%d", ov.Done, ov.Total)
	line(bar(ov.Frac(), w-len(count)) + sDim.Render(count))

	cur, ok := m.app.Current()
	doing := []app.Ref{}
	if !ok {
		heading("ALL DONE")
		line("Every step on the main path is complete.")
	} else {
		heading("NOW")
		m.renderFocus(cur, w, line, block)
		if ref, _ := m.selected(); m.sel > 0 {
			heading("SELECTED")
			m.renderFocus(ref, w, line, block)
		}
		doing = m.app.Doing(cur.Step.ID)
		if len(doing) > 0 {
			heading("ALSO IN PROGRESS")
			for i, r := range doing {
				item(1+i, r)
			}
		}
	}

	out = &bottom
	if done := m.app.RecentDone(3); len(done) > 0 {
		heading("DONE")
		for _, r := range done {
			line(statusIcon(store.Done) + " " + sDim.Render(r.Step.Title))
		}
	}

	// NEXT gets whatever height is left.
	var next []string
	m.nextN = 0
	if avail := m.h - 1 - len(top) - len(bottom) - 2; ok && avail > 0 {
		out = &next
		heading("NEXT")
		lastSec := cur.Sec.ID
		for _, r := range m.app.Upcoming(avail) {
			if r.Sec.ID != lastSec {
				if len(next) >= avail { // no room for header + an item

					break
				}
				line(sDim.Render(r.Sec.Title))
				lastSec = r.Sec.ID
			}
			if len(next) >= avail+2 {
				break
			}
			item(1+len(doing)+m.nextN, r)
			m.nextN++
		}
	}

	lines := append(append(top, next...), bottom...)
	if len(lines) > m.h-1 {
		lines = lines[:m.h-1]
	}
	for len(lines) < m.h-1 {
		lines = append(lines, "")
	}
	return strings.Join(lines, "\n") + "\n" + m.footer(w)
}

func (m *Side) renderFocus(r app.Ref, w int, line func(string), block func(string, int)) {
	p := m.app.Progress
	st := r.Step
	block(statusIcon(p.StatusOf(st.ID))+" "+sBold.Render(st.Title), 0)
	meta := []string{kindTag(st.Kind)}
	if st.Hours > 0 {
		meta = append(meta, fmt.Sprintf("~%gh", st.Hours))
	}
	if mins := p.Minutes[st.ID]; mins > 0 {
		meta = append(meta, sWarn.Render(store.FormatMinutes(mins)+" logged"))
	}
	if st.Platform != "" {
		meta = append(meta, sWarn.Render(st.Platform))
	}
	line("  " + strings.Join(meta, sDim.Render(" · ")))
	line("  " + sDim.Render(r.Sec.Title))
	if t := m.app.Timer; t != nil && t.ID == st.ID {
		line("  " + sWarn.Render("◷ "+clock(t.Elapsed(time.Now()))+" running"))
	}
	if st.Detail != "" {
		lines := strings.Split(wrap(reflow(st.Detail), w-2), "\n")
		if len(lines) > 6 {
			lines = append(lines[:5], "… (full text: enter in `sysroad`)")
		}
		for _, l := range lines {
			line("  " + sDim.Render(l))
		}
	}
	if st.DoneWhen != "" {
		line("  " + sGood.Bold(true).Render("Done when"))
		for _, l := range strings.Split(clip(wrap(reflow(st.DoneWhen), w-2), 5), "\n") {
			line("  " + l)
		}
	}
	if note := p.Notes[st.ID]; note != "" {
		line("  " + sHeading.Render("✎ notes"))
		for _, l := range strings.Split(clip(wrap(note, w-2), 6), "\n") {
			line("  " + l)
		}
	}
}

func (m *Side) footer(w int) string {
	if m.flash != "" {
		st := sGood
		if m.flashErr {
			st = sErr
		}
		return " " + trunc(st.Render(m.flash), w)
	}
	hints := keyHints([][2]string{{"spc", "done"}, {"t", "timer"}, {"n", "note"}, {"j/k", "pick"}, {"o", "open"}, {"q", ""}})
	return trunc(spread(hints, m.syncStatus(), w+1), w+1)
}
