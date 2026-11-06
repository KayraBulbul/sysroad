package tui

import (
	"strings"

	"github.com/alecthomas/chroma/v2"
	"github.com/alecthomas/chroma/v2/lexers"
	"github.com/charmbracelet/lipgloss"
	"github.com/charmbracelet/x/ansi"
)

// RenderMarkdown renders the small markdown subset lessons use, wrapped to
// width w, using only the terminal's 16 palette colours:
// headings, paragraphs, lists and checklists, > callouts, fenced code
// (C++ is highlighted), `code`, **bold**, *italic* and [links](url).
func RenderMarkdown(md string, w int) string {
	if w < 10 {
		w = 10
	}
	var out []string
	lines := strings.Split(strings.ReplaceAll(md, "\r\n", "\n"), "\n")
	var para []string
	flush := func() {
		if len(para) > 0 {
			out = append(out, wrapLines(inline(strings.Join(para, " ")), w, "", "")...)
			para = nil
		}
	}
	blank := func() {
		if len(out) > 0 && out[len(out)-1] != "" {
			out = append(out, "")
		}
	}
	for i := 0; i < len(lines); i++ {
		line := lines[i]
		trim := strings.TrimSpace(line)
		switch {
		case strings.HasPrefix(trim, "```"):
			flush()
			lang := strings.TrimSpace(strings.TrimPrefix(trim, "```"))
			var code []string
			for i++; i < len(lines) && !strings.HasPrefix(strings.TrimSpace(lines[i]), "```"); i++ {
				code = append(code, lines[i])
			}
			out = append(out, codeBlock(strings.Join(code, "\n"), lang, w)...)
			blank()
		case trim == "":
			flush()
			blank()
		case strings.HasPrefix(trim, "#"):
			flush()
			level := len(trim) - len(strings.TrimLeft(trim, "#"))
			text := strings.TrimSpace(trim[level:])
			blank()
			switch level {
			case 1:
				out = append(out, wrapLines(sAccent.Render(text), w, "", "")...)
			case 2:
				out = append(out, wrapLines(sHeading.Render(text), w, "", "")...)
			default:
				out = append(out, wrapLines(sBold.Render(text), w, "", "")...)
			}
		case strings.HasPrefix(trim, ">"):
			flush()
			var q []string
			for ; i < len(lines) && strings.HasPrefix(strings.TrimSpace(lines[i]), ">"); i++ {
				q = append(q, strings.TrimSpace(strings.TrimPrefix(strings.TrimSpace(lines[i]), ">")))
			}
			i--
			bar := sWarn.Render("▌ ")
			out = append(out, wrapLines(inline(strings.Join(q, " ")), w, bar, bar)...)
		case isListItem(trim):
			flush()
			marker, text := listMarker(trim)
			// Indented lines that follow continue the item.
			for i+1 < len(lines) && strings.HasPrefix(lines[i+1], "  ") && strings.TrimSpace(lines[i+1]) != "" && !isListItem(strings.TrimSpace(lines[i+1])) {
				i++
				text += " " + strings.TrimSpace(lines[i])
			}
			out = append(out, wrapLines(inline(text), w, marker, strings.Repeat(" ", ansi.StringWidth(marker)))...)
		default:
			para = append(para, trim)
		}
	}
	flush()
	for len(out) > 0 && out[len(out)-1] == "" {
		out = out[:len(out)-1]
	}
	return strings.Join(out, "\n")
}

func isListItem(s string) bool {
	if strings.HasPrefix(s, "- ") || strings.HasPrefix(s, "* ") {
		return true
	}
	n := 0
	for n < len(s) && s[n] >= '0' && s[n] <= '9' {
		n++
	}
	return n > 0 && n < len(s)-1 && s[n] == '.' && s[n+1] == ' '
}

func listMarker(s string) (marker, text string) {
	switch {
	case strings.HasPrefix(s, "- [ ] "):
		return sDim.Render("☐ "), s[6:]
	case strings.HasPrefix(s, "- [x] "), strings.HasPrefix(s, "- [X] "):
		return sGood.Render("☑ "), s[6:]
	case strings.HasPrefix(s, "- "), strings.HasPrefix(s, "* "):
		return sDim.Render("• "), s[2:]
	}
	dot := strings.IndexByte(s, '.')
	return sDim.Render(s[:dot+1]) + " ", s[dot+2:]
}

// wrapLines word-wraps styled text with a first-line and continuation prefix.
func wrapLines(s string, w int, first, rest string) []string {
	pw := max(ansi.StringWidth(first), ansi.StringWidth(rest))
	wrapped := strings.Split(ansi.Wrap(s, max(w-pw, 4), ""), "\n")
	for i := range wrapped {
		if i == 0 {
			wrapped[i] = first + wrapped[i]
		} else {
			wrapped[i] = rest + wrapped[i]
		}
	}
	return wrapped
}

var (
	sCode = lipgloss.NewStyle().Foreground(cCyan)
	sLink = lipgloss.NewStyle().Foreground(cBlue).Underline(true)
	sItal = lipgloss.NewStyle().Italic(true)
)

// inline renders `code`, **bold**, *italic* and [text](url) spans.
func inline(s string) string {
	var b strings.Builder
	for i := 0; i < len(s); {
		switch {
		case s[i] == '`':
			if j := strings.IndexByte(s[i+1:], '`'); j >= 0 {
				b.WriteString(sCode.Render(s[i+1 : i+1+j]))
				i += j + 2
				continue
			}
		case strings.HasPrefix(s[i:], "**"):
			if j := strings.Index(s[i+2:], "**"); j > 0 {
				b.WriteString(sBold.Render(stripMarks(s[i+2 : i+2+j])))
				i += j + 4
				continue
			}
		case s[i] == '*' && i+1 < len(s) && s[i+1] != ' ':
			if j := strings.IndexByte(s[i+1:], '*'); j > 0 {
				b.WriteString(sItal.Render(stripMarks(s[i+1 : i+1+j])))
				i += j + 2
				continue
			}
		case s[i] == '[':
			if j := strings.Index(s[i:], "]("); j > 0 {
				if k := strings.IndexByte(s[i+j:], ')'); k > 0 {
					text, url := s[i+1:i+j], s[i+j+2:i+j+k]
					b.WriteString(hyperlink(url, sLink.Render(stripMarks(text))))
					i += j + k + 1
					continue
				}
			}
		}
		b.WriteByte(s[i])
		i++
	}
	return b.String()
}

func stripMarks(s string) string { return strings.NewReplacer("`", "", "**", "").Replace(s) }

// hyperlink wraps text in an OSC 8 link, which Ghostty, kitty and most
// modern terminals make clickable; others just show the text.
func hyperlink(url, text string) string {
	return "\x1b]8;;" + url + "\x1b\\" + text + "\x1b]8;;\x1b\\"
}

// codeBlock renders fenced code with a gutter. C and C++ are highlighted;
// anything else (diagrams, shell output) is shown as is.
func codeBlock(code, lang string, w int) []string {
	gutter := sDim.Render("│ ")
	var styled []string
	switch strings.ToLower(lang) {
	case "cpp", "c++", "c", "cc", "h", "hpp":
		styled = highlight(code, "cpp")
	default:
		styled = strings.Split(code, "\n")
	}
	var out []string
	for _, l := range styled {
		l = strings.ReplaceAll(l, "\t", "    ")
		for i, part := range strings.Split(ansi.Hardwrap(l, max(w-2, 4), true), "\n") {
			if i > 0 {
				out = append(out, sDim.Render("│↪")+part)
			} else {
				out = append(out, gutter+part)
			}
		}
	}
	return out
}

// tokenStyle maps chroma token types onto palette colours.
func tokenStyle(t chroma.TokenType) (lipgloss.Style, bool) {
	switch {
	case t == chroma.CommentPreproc || t == chroma.CommentPreprocFile:
		return lipgloss.NewStyle().Foreground(cMagenta), true
	case t.InCategory(chroma.Comment):
		return lipgloss.NewStyle().Foreground(cDim).Italic(true), true
	case t == chroma.KeywordType:
		return lipgloss.NewStyle().Foreground(cYellow), true
	case t.InCategory(chroma.Keyword):
		return lipgloss.NewStyle().Foreground(cMagenta), true
	case t.InSubCategory(chroma.LiteralString):
		return lipgloss.NewStyle().Foreground(cGreen), true
	case t.InSubCategory(chroma.LiteralNumber):
		return lipgloss.NewStyle().Foreground(cYellow), true
	case t == chroma.NameFunction:
		return lipgloss.NewStyle().Foreground(cBlue), true
	case t == chroma.NameBuiltin || t == chroma.NameNamespace:
		return lipgloss.NewStyle().Foreground(cCyan), true
	}
	return lipgloss.Style{}, false
}

func highlight(code, lang string) []string {
	lx := lexers.Get(lang)
	if lx == nil {
		return strings.Split(code, "\n")
	}
	it, err := chroma.Coalesce(lx).Tokenise(nil, code)
	if err != nil {
		return strings.Split(code, "\n")
	}
	lines := []string{""}
	for _, tok := range it.Tokens() {
		st, ok := tokenStyle(tok.Type)
		for i, part := range strings.Split(tok.Value, "\n") {
			if i > 0 {
				lines = append(lines, "")
			}
			if part == "" {
				continue
			}
			if ok {
				part = st.Render(part)
			}
			lines[len(lines)-1] += part
		}
	}
	// Tokenise adds a trailing newline.
	if len(lines) > 1 && lines[len(lines)-1] == "" && !strings.HasSuffix(code, "\n") {
		lines = lines[:len(lines)-1]
	}
	return lines
}
