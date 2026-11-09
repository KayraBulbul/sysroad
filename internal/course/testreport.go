package course

import (
	"bufio"
	"bytes"
	"encoding/json"
	"fmt"
	"path/filepath"
	"strconv"
	"strings"
	"unicode/utf8"
)

// The test runner's reporter (third_party/sysroad_reporter.hpp) writes one
// JSON object per line. This file reads that back and lays it out: one line
// per test case, and for each failure the two values on their own lines with
// the part that differs highlighted.

type testEvent struct {
	Ev        string   `json:"ev"`
	Name      string   `json:"name"`
	File      string   `json:"file"`
	Line      int      `json:"line"`
	OK        bool     `json:"ok"`
	Crash     bool     `json:"crash"`
	Text      string   `json:"text"`
	Type      string   `json:"type"`
	Expr      string   `json:"expr"`
	Decomp    string   `json:"decomp"`
	Threw     bool     `json:"threw"`
	Exception string   `json:"exception"`
	ExcType   string   `json:"exc_type"`
	ExcString string   `json:"exc_string"`
	Subcases  []string `json:"subcases"`
	Info      []string `json:"info"`
}

type testCase struct {
	name, file string
	line       int
	started    bool
	ended      bool
	ok         bool
	failures   []testEvent // assert and message events
	exception  *testEvent  // thrown out of the test case, or a crash doctest caught
}

type testReport struct {
	cases []*testCase // in the order they run
	done  bool        // the reporter wrote its last line
}

// crashed reports whether the run ended early: killed outright, or a crash
// doctest caught (it still finishes the report, then exits).
func (r *testReport) crashed() bool {
	if !r.done {
		return true
	}
	for _, tc := range r.cases {
		if tc.exception != nil && tc.exception.Crash {
			return true
		}
	}
	return false
}

func parseTestReport(data []byte) *testReport {
	r := &testReport{}
	byKey := map[string]*testCase{}
	var cur *testCase
	sc := bufio.NewScanner(bytes.NewReader(data))
	sc.Buffer(make([]byte, 64<<10), 4<<20)
	for sc.Scan() {
		var ev testEvent
		if json.Unmarshal(sc.Bytes(), &ev) != nil {
			continue // a line cut short by a crash
		}
		switch ev.Ev {
		case "case", "start":
			key := fmt.Sprintf("%s\x00%s\x00%d", ev.Name, ev.File, ev.Line)
			tc := byKey[key]
			if tc == nil {
				tc = &testCase{name: ev.Name, file: ev.File, line: ev.Line}
				byKey[key] = tc
				r.cases = append(r.cases, tc)
			}
			if ev.Ev == "start" {
				tc.started, cur = true, tc
			}
		case "assert", "message":
			if cur != nil {
				cur.failures = append(cur.failures, ev)
			}
		case "exception":
			if cur != nil {
				cur.exception = &ev
			}
		case "end":
			if cur != nil {
				cur.ended, cur.ok = true, ev.OK
			}
			cur = nil
		case "done":
			r.done = true
		}
	}
	return r
}

// counts are the test cases that ran, and how many of those passed.
func (r *testReport) counts() (passed, total int) {
	for _, tc := range r.cases {
		if tc.started {
			total++
			if tc.ended && tc.ok {
				passed++
			}
		}
	}
	return passed, total
}

// maxPassedListed is how many passing test cases are listed by name before
// they're folded into one line.
const maxPassedListed = 10

// maxDetailed is how many failing test cases get their details; the rest
// are one line each.
const maxDetailed = 3

// render lays out a failed run. printed is what the tests wrote to stdout,
// report the tidied stderr (often a sanitizer report), and note why the
// runner stopped, if it was killed.
func (r *testReport) render(mode Mode, printed, report, note string) string {
	var b strings.Builder
	var passed, failed, notRun []*testCase
	for _, tc := range r.cases {
		switch {
		case !tc.started:
			notRun = append(notRun, tc)
		case tc.ended && tc.ok:
			passed = append(passed, tc)
		default:
			failed = append(failed, tc)
		}
	}
	counts := []string{fmt.Sprintf("%d passed", len(passed)), fmt.Sprintf("%d failed", len(failed))}
	if len(notRun) > 0 {
		counts = append(counts, fmt.Sprintf("%d not run", len(notRun)))
	}
	b.WriteString(section("Tests", strings.Join(counts, " · ")))

	if len(passed) > maxPassedListed {
		b.WriteString(green("✓") + dim(fmt.Sprintf(" %d test cases passed", len(passed))) + "\n")
	}
	detailed, gap := 0, false
	for _, tc := range r.cases {
		if gap && tc.started {
			b.WriteString("\n") // a blank line after a failure's details
			gap = false
		}
		switch {
		case !tc.started:
		case tc.ended && tc.ok:
			if len(passed) <= maxPassedListed {
				b.WriteString(green("✓") + " " + dim(tc.name) + "\n")
			}
		case detailed < maxDetailed:
			detailed++
			b.WriteString(renderFailedCase(tc, r.done, note))
			gap = true
		default:
			b.WriteString(red("✗") + " " + tc.name + "  " + dim(caseWhere(tc)) + "\n")
		}
	}

	if len(notRun) > 0 {
		b.WriteString("\n")
		for i, tc := range notRun {
			if i == 3 {
				b.WriteString(dim(fmt.Sprintf("· … and %d more", len(notRun)-3)) + "\n")
				break
			}
			b.WriteString(dim("· "+tc.name+" — not run") + "\n")
		}
		why := "Run stops at the first failure; Submit runs them all"
		if r.crashed() {
			why = "the run stopped when the tests crashed"
		}
		b.WriteString(dim("  ("+why+")") + "\n")
	}

	if s := strings.TrimRight(printed, "\n"); strings.TrimSpace(s) != "" {
		b.WriteString("\n" + dim("printed during the tests:") + "\n" + indent(clipLines(s, 30), "  ") + "\n")
	}
	if s := strings.TrimSpace(report); s != "" {
		b.WriteString("\n" + s + "\n")
	}
	return b.String()
}

func renderFailedCase(tc *testCase, done bool, note string) string {
	var b strings.Builder
	b.WriteString(red("✗") + " " + bold(tc.name) + "  " + dim(caseWhere(tc)) + "\n")

	const maxShown = 3
	for i, f := range tc.failures {
		if i == maxShown {
			b.WriteString(dim(fmt.Sprintf("  … and %d more failed checks in this test", len(tc.failures)-maxShown)) + "\n")
			break
		}
		if i > 0 {
			b.WriteString("\n" + dim("  "+filepath.Base(f.File)+":"+fmt.Sprint(f.Line)) + "\n")
		}
		b.WriteString(renderFailure(f))
	}
	switch {
	case tc.exception != nil && tc.exception.Crash:
		b.WriteString("  " + red("crashed: ") + tc.exception.Text + "\n")
	case tc.exception != nil:
		b.WriteString("  " + red("threw an exception: ") + quotedText(tc.exception.Text) + "\n")
	case !tc.ended && note != "":
		b.WriteString("  " + red("didn't finish"+note) + "\n")
	case !tc.ended && !done:
		b.WriteString("  " + red("crashed here") + dim(" — the report below says where") + "\n")
	}
	return b.String()
}

// caseWhere is where a failed test case went wrong: its first failure, or
// the test case itself.
func caseWhere(tc *testCase) string {
	if len(tc.failures) > 0 {
		f := tc.failures[0]
		return filepath.Base(f.File) + ":" + fmt.Sprint(f.Line)
	}
	return filepath.Base(tc.file) + ":" + fmt.Sprint(tc.line)
}

func renderFailure(f testEvent) string {
	var b strings.Builder
	line := func(s string) { b.WriteString("  " + s + "\n") }
	for _, s := range f.Subcases {
		line(dim("in subcase: " + s))
	}
	if f.Ev == "message" {
		for _, s := range f.Info {
			line(dim(s))
		}
		line(unquote(f.Text))
		return b.String()
	}
	expr := f.Expr
	if strings.Contains(f.Type, "THROWS_AS") && f.ExcType != "" {
		expr += ", " + f.ExcType
	}
	line(dim(f.Type + "(" + expr + ")"))
	for _, s := range f.Info {
		line(dim(s))
	}

	var detail string
	switch {
	case strings.Contains(f.Type, "THROWS"):
		want := f.ExcType
		if want == "" && f.ExcString != "" {
			want = "an exception saying " + f.ExcString
		}
		switch {
		case !f.Threw && want != "":
			detail = "  " + red("didn't throw") + " — expected " + want + "\n"
		case !f.Threw:
			detail = "  " + red("didn't throw") + "\n"
		default:
			detail = "  " + red("threw ") + quotedText(f.Exception) + " — expected " + want + "\n"
		}
	case f.Threw:
		detail = "  " + red("threw an exception: ") + quotedText(f.Exception) + "\n"
	default:
		detail = renderValues(f.Expr, f.Decomp)
	}
	if detail != "" {
		b.WriteString("\n" + detail)
	}
	return b.String()
}

// renderValues shows what an assert compared. For a == it puts the two
// values on their own lines, labelled with your code's name for the value
// and "expected", with the part that differs highlighted.
func renderValues(expr, decomp string) string {
	gotV, wantV, ok := splitTop(decomp, " == ")
	if !ok {
		switch decomp {
		case "", "true", "false":
			return "" // the expression line already says it
		}
		return "  " + dim("values: ") + decomp + "\n"
	}
	gotL, wantL := "left", "right"
	if l, r, ok := splitTop(expr, " == "); ok {
		l, r = strings.TrimSpace(l), strings.TrimSpace(r)
		switch {
		case isLiteral(r) && !isLiteral(l):
			gotL, wantL = l, "expected"
		case isLiteral(l) && !isLiteral(r):
			gotL, wantL = r, "expected"
			gotV, wantV = wantV, gotV
		case !isLiteral(l) && !isLiteral(r):
			gotL, wantL = l, r
		}
	}
	if utf8.RuneCountInString(gotL) > 24 || utf8.RuneCountInString(wantL) > 24 {
		if wantL == "expected" {
			gotL = "got"
		} else {
			gotL, wantL = "left", "right"
		}
	}
	w := max(utf8.RuneCountInString(gotL), utf8.RuneCountInString(wantL)) + 2
	pad := func(s string) string { return s + strings.Repeat(" ", w-utf8.RuneCountInString(s)) }

	gotOut, wantOut, col, diffed := diffValues(gotV, wantV)
	s := "  " + pad(gotL) + gotOut + "\n" + "  " + pad(wantL) + wantOut + "\n"
	// Only worth an arrow when the values start the same.
	if diffed && col > 1 {
		s += "  " + strings.Repeat(" ", w+col) + dim("↑ differs from here") + "\n"
	}
	return s
}

// splitTop splits s at the first sep that isn't inside quotes or brackets.
func splitTop(s, sep string) (left, right string, ok bool) {
	depth := 0
	for i := 0; i < len(s); i++ {
		switch c := s[i]; c {
		case '"':
			for i++; i < len(s) && s[i] != '"'; i++ {
				if s[i] == '\\' {
					i++
				}
			}
		case '\'':
			// A char literal: 'a' or '\n'.
			if j := strings.IndexByte(s[i+1:], '\''); j >= 0 && j <= 4 {
				i += j + 1
			}
		case '(', '[', '{':
			depth++
		case ')', ']', '}':
			depth--
		default:
			if depth == 0 && strings.HasPrefix(s[i:], sep) {
				return s[:i], s[i+len(sep):], true
			}
		}
	}
	return "", "", false
}

// isLiteral reports whether a bit of C++ is a value written out, like "hi",
// 42, -1, 'c', {1, 2} or std::vector<int>{1, 2}.
func isLiteral(s string) bool {
	if s == "" {
		return false
	}
	switch s {
	case "true", "false", "nullptr":
		return true
	}
	c := s[0]
	switch {
	case c == '"' || c == '\'' || c == '{' || (c >= '0' && c <= '9'):
		return true
	case c == '-' && len(s) > 1 && s[1] >= '0' && s[1] <= '9':
		return true
	case strings.HasPrefix(s, `R"`) || strings.HasPrefix(s, `u8"`):
		return true
	case strings.HasPrefix(s, "std::") && strings.HasSuffix(s, "}") && strings.Contains(s, "{"):
		return true
	}
	return false
}

// diffValues highlights where two strings (or two {…} lists) differ: the
// common start and end stay plain, the middle is coloured. col is how far
// into the value the first difference is, for the arrow under it.
func diffValues(got, want string) (gotOut, wantOut string, col int, ok bool) {
	quoted := func(s string) bool { return len(s) >= 2 && s[0] == '"' && s[len(s)-1] == '"' }
	list := func(s string) bool { return len(s) >= 2 && s[0] == '{' && s[len(s)-1] == '}' }
	if !(quoted(got) && quoted(want)) && !(list(got) && list(want)) {
		return got, want, 0, false
	}
	g, w := units(got), units(want)
	p := 0
	for p < len(g) && p < len(w) && g[p] == w[p] {
		p++
	}
	s := 0
	for s < len(g)-p && s < len(w)-p && g[len(g)-1-s] == w[len(w)-1-s] {
		s++
	}
	join := func(u []string) string { return strings.Join(u, "") }
	gotOut = join(g[:p]) + hl(join(g[p:len(g)-s]), "41") + join(g[len(g)-s:])
	wantOut = join(w[:p]) + hl(join(w[p:len(w)-s]), "42") + join(w[len(w)-s:])
	return gotOut, wantOut, utf8.RuneCountInString(join(g[:p])), true
}

// units splits a value as it's shown into what reads as one character: an
// escape like \n or \x1b, or one rune.
func units(s string) []string {
	var u []string
	for i := 0; i < len(s); {
		n := 1
		if s[i] == '\\' && i+1 < len(s) {
			n = 2
			if s[i+1] == 'x' && i+4 <= len(s) {
				n = 4
			}
		} else {
			_, n = utf8.DecodeRuneInString(s[i:])
		}
		u = append(u, s[i:i+n])
		i += n
	}
	return u
}

// hl puts a background colour (41 red, 42 green) behind s.
func hl(s, bg string) string {
	if s == "" {
		return ""
	}
	return "\x1b[" + bg + ";97m" + s + "\x1b[49;39m"
}

func bold(s string) string { return "\x1b[1m" + s + "\x1b[22m" }

// section starts a block of output with a dim, underlined title (the GUI
// draws it as a heading with a rule) and optional notes after it.
func section(title, notes string) string {
	s := "\x1b[90;4m" + strings.ToUpper(title) + "\x1b[24;39m"
	if notes != "" {
		s += dim("  " + notes)
	}
	return s + "\n"
}

// unquote turns "text" (how the tests show a string) back into text.
func unquote(s string) string {
	if u, err := strconv.Unquote(s); err == nil && strings.HasPrefix(s, `"`) {
		return u
	}
	return s
}

// quotedText shows an exception's message in quotes, whether or not doctest
// already quoted it.
func quotedText(s string) string {
	if strings.HasPrefix(s, `"`) && strings.HasSuffix(s, `"`) && len(s) >= 2 {
		return s
	}
	return visible(s)
}

func clipLines(s string, n int) string {
	lines := strings.Split(s, "\n")
	if len(lines) <= n {
		return s
	}
	return strings.Join(lines[:n], "\n") + "\n" + dim(fmt.Sprintf("… %d more lines", len(lines)-n))
}
