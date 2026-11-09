package course

import (
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func TestRenderValuesLabelsAndHighlights(t *testing.T) {
	for _, tc := range []struct {
		name, expr, decomp string
		want               string // without colours
		gotHL, wantHL      string // the highlighted parts, "" for none
	}{{
		name:   "your value first, the literal is expected",
		expr:   `r.out == "  1 a.txt\n"`,
		decomp: `"  0\n" == "  1 a.txt\n"`,
		want: `  r.out     "  0\n"
  expected  "  1 a.txt\n"
               ↑ differs from here
`,
		gotHL: "0", wantHL: "1 a.txt",
	}, {
		name:   "a literal on the left is still the expected one",
		expr:   `4 == n`,
		decomp: `4 == 3`,
		want: `  n         3
  expected  4
`,
	}, {
		name:   "== inside a string isn't the comparison",
		expr:   `s == "a == b"`,
		decomp: `"a = b" == "a == b"`,
		want: `  s         "a = b"
  expected  "a == b"
                ↑ differs from here
`,
		wantHL: "=",
	}, {
		name:   "an escape is one character",
		expr:   `s == "ab\n"`,
		decomp: `"ab\t" == "ab\n"`,
		want: `  s         "ab\t"
  expected  "ab\n"
               ↑ differs from here
`,
		gotHL: `\t`, wantHL: `\n`,
	}, {
		name:   "two variables keep their names",
		expr:   `got == want`,
		decomp: `{1, 2} == {1, 3}`,
		want: `  got   {1, 2}
  want  {1, 3}
            ↑ differs from here
`,
		gotHL: "2", wantHL: "3",
	}, {
		name:   "other comparisons show both values",
		expr:   `n < 10`,
		decomp: `12 < 10`,
		want:   "  values: 12 < 10\n",
	}} {
		t.Run(tc.name, func(t *testing.T) {
			raw := renderValues(tc.expr, tc.decomp)
			if got := stripANSI(raw); got != tc.want {
				t.Errorf("got\n%s\nwant\n%s", got, tc.want)
			}
			for _, h := range []struct{ bg, text string }{{"41", tc.gotHL}, {"42", tc.wantHL}} {
				marked := "\x1b[" + h.bg + ";97m" + h.text + "\x1b[49;39m"
				if h.text != "" && !strings.Contains(raw, marked) {
					t.Errorf("%q isn't highlighted in %q", h.text, raw)
				}
				if h.text == "" && strings.Contains(raw, "\x1b["+h.bg+";") {
					t.Errorf("unexpected highlight in %q", raw)
				}
			}
		})
	}
}

// A real doctest run through sysroad's reporter: failures, a passing test,
// and a crash that kills the runner before the last test.
func TestTestReportFromARealRun(t *testing.T) {
	needCompiler(t)
	cxx, _ := Compiler()
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Minute)
	defer cancel()
	inc, obj, err := e.doctest(ctx, cxx)
	if err != nil {
		t.Fatal(err)
	}
	dir := t.TempDir()
	src := filepath.Join(dir, "visible_test.cpp")
	os.WriteFile(src, []byte(`#include "doctest.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
static std::string wc() { return "      0\n"; }
static int boom() { throw std::runtime_error("bad input"); }
TEST_CASE("a passing test") { CHECK(1 + 1 == 2); }
TEST_CASE("one file, named") {
    std::cout << "debug print" << std::endl;
    CHECK(wc() == "      1 a.txt\n");
}
TEST_CASE("throws") { CHECK(boom() == 1); }
TEST_CASE("crashes") { std::abort(); }
TEST_CASE("never reached") { CHECK(true); }
`), 0o644)
	flags, _ := e.flags(ctx, cxx)
	bin := filepath.Join(dir, "tests")
	args := append(append([]string{}, flags...), "-w", "-isystem", inc,
		"-include", filepath.Join(inc, "sysroad_doctest.hpp"), src, obj, "-o", bin)
	if out, err := exec.CommandContext(ctx, cxx, args...).CombinedOutput(); err != nil {
		t.Fatalf("%v\n%s", err, out)
	}

	res, rep := e.runTests(ctx, bin, ModeSubmit)
	if res.code == 0 {
		t.Fatal("the run should fail")
	}
	if !rep.crashed() {
		t.Error("the run crashed, but the report says it finished")
	}
	if passed, total := rep.counts(); passed != 1 || total != 4 {
		t.Errorf("counts = %d/%d, want 1/4", passed, total)
	}
	got := stripANSI(rep.render(ModeSubmit, res.stdout, "", res.note))
	for _, want := range []string{
		"TESTS  1 passed · 3 failed · 1 not run\n",
		"✓ a passing test\n",
		"✗ one file, named  visible_test.cpp:11\n" +
			`  CHECK(wc() == "      1 a.txt\n")` + "\n\n" +
			`  wc()      "      0\n"` + "\n" +
			`  expected  "      1 a.txt\n"` + "\n" +
			"                   ↑ differs from here\n",
		`  threw an exception: "bad input"`,
		"✗ crashes  visible_test.cpp:14\n",
		"· never reached — not run\n  (the run stopped when the tests crashed)\n",
		"printed during the tests:\n  debug print\n",
	} {
		if !strings.Contains(got, want) {
			t.Errorf("output lacks\n%s\n--- output:\n%s", want, got)
		}
	}
}
