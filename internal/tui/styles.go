package tui

import (
	"strings"

	"github.com/charmbracelet/lipgloss"
	"github.com/charmbracelet/x/ansi"

	"github.com/KayraBulbul/sysroad/internal/roadmap"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// ANSI palette colours (0–15) so sysroad follows the terminal theme.
var (
	cRed     = lipgloss.Color("1")
	cGreen   = lipgloss.Color("2")
	cYellow  = lipgloss.Color("3")
	cBlue    = lipgloss.Color("4")
	cMagenta = lipgloss.Color("5")
	cCyan    = lipgloss.Color("6")
	cDim     = lipgloss.Color("8")

	sBold    = lipgloss.NewStyle().Bold(true)
	sDim     = lipgloss.NewStyle().Foreground(cDim)
	sAccent  = lipgloss.NewStyle().Foreground(cBlue).Bold(true)
	sHeading = lipgloss.NewStyle().Foreground(cMagenta).Bold(true)
	sGood    = lipgloss.NewStyle().Foreground(cGreen)
	sWarn    = lipgloss.NewStyle().Foreground(cYellow)
	sErr     = lipgloss.NewStyle().Foreground(cRed)
	sCursor  = lipgloss.NewStyle().Reverse(true)
	sKey     = lipgloss.NewStyle().Foreground(cCyan).Bold(true)
)

func statusIcon(s store.Status) string {
	switch s {
	case store.Done:
		return sGood.Render("✓")
	case store.Doing:
		return sWarn.Render("◐")
	case store.Skipped:
		return sDim.Render("–")
	default:
		return sDim.Render("○")
	}
}

var kindStyle = map[string]lipgloss.Style{
	roadmap.KindSetup:    lipgloss.NewStyle().Foreground(cDim),
	roadmap.KindRead:     lipgloss.NewStyle().Foreground(cBlue),
	roadmap.KindExercise: lipgloss.NewStyle().Foreground(cCyan),
	roadmap.KindLab:      lipgloss.NewStyle().Foreground(cYellow),
	roadmap.KindProject:  lipgloss.NewStyle().Foreground(cGreen),
	roadmap.KindCapstone: lipgloss.NewStyle().Foreground(cMagenta).Bold(true),
}

var kindShort = map[string]string{
	roadmap.KindSetup: "set", roadmap.KindRead: "read", roadmap.KindExercise: "ex",
	roadmap.KindLab: "lab", roadmap.KindProject: "proj", roadmap.KindCapstone: "CAP",
}

// kindTag renders a fixed-width (4 col) kind label.
func kindTag(kind string) string {
	short, ok := kindShort[kind]
	if !ok {
		short = kind
		if short == "" {
			short = "task"
		}
	}
	short = ansi.Truncate(short, 4, "")
	st, ok := kindStyle[kind]
	if !ok {
		st = sDim
	}
	return st.Render(pad(short, 4))
}

func bar(frac float64, width int) string {
	if width < 1 {
		return ""
	}
	n := int(frac*float64(width) + 0.5)
	n = min(max(n, 0), width)
	return sAccent.Render(strings.Repeat("━", n)) + sDim.Render(strings.Repeat("━", width-n))
}

func pad(s string, w int) string {
	if d := w - ansi.StringWidth(s); d > 0 {
		return s + strings.Repeat(" ", d)
	}
	return s
}

func trunc(s string, w int) string {
	if w <= 0 {
		return ""
	}
	return ansi.Truncate(s, w, "…")
}

// wrap word-wraps plain or styled text to width w.
func wrap(s string, w int) string {
	if w <= 0 {
		return ""
	}
	return ansi.Wrap(s, w, "")
}

// spread puts left and right on one line of width w, truncating left.
func spread(left, right string, w int) string {
	rw := ansi.StringWidth(right)
	left = trunc(left, w-rw-1)
	gap := w - ansi.StringWidth(left) - rw
	if gap < 1 {
		gap = 1
	}
	return left + strings.Repeat(" ", gap) + right
}

// clip keeps at most n lines of s.
func clip(s string, n int) string {
	if n <= 0 {
		return ""
	}
	lines := strings.Split(s, "\n")
	if len(lines) > n {
		lines = lines[:n]
	}
	return strings.Join(lines, "\n")
}

// window returns [start,end) of a scrolled list keeping cursor visible.
func window(n, cursor, visible int) (int, int) {
	if visible <= 0 {
		return 0, 0
	}
	if n <= visible {
		return 0, n
	}
	start := cursor - visible/2
	start = min(max(start, 0), n-visible)
	return start, start + visible
}

func stripStyle(s string) string { return ansi.Strip(s) }

// reflow joins hard-wrapped prose lines so they rewrap at the pane width,
// but leaves paragraphs that look like lists or aligned columns untouched.
func reflow(s string) string {
	paras := strings.Split(s, "\n\n")
	for i, p := range paras {
		lines := strings.Split(p, "\n")
		structured := false
		for _, l := range lines {
			t := strings.TrimLeft(l, " ")
			if len(t) != len(l) || strings.HasPrefix(t, "- ") || strings.HasPrefix(t, "* ") ||
				strings.HasPrefix(t, "• ") || strings.Contains(t, "  ") {
				structured = true
				break
			}
		}
		if !structured {
			var b strings.Builder
			for j, l := range lines {
				b.WriteString(l)
				// "length-\nprefixed" rejoins as "length-prefixed".
				if j < len(lines)-1 && !strings.HasSuffix(l, "-") {
					b.WriteByte(' ')
				}
			}
			paras[i] = b.String()
		}
	}
	return strings.Join(paras, "\n\n")
}
