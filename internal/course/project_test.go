package course

import (
	"context"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"testing/fstest"
	"time"
)

// tally is a three-lesson fixture project (testdata/tally): lesson 2 adds
// a type and a source in a subfolder, lesson 3 changes a signature, drops a
// function, deletes a file and retires a test file, a test case and a check.
func tally(t *testing.T) *Course {
	t.Helper()
	c, err := LoadFS(os.DirFS("testdata/tally"))
	if err != nil {
		t.Fatal(err)
	}
	if err := c.Validate(); err != nil {
		t.Fatal(err)
	}
	return c
}

func TestFixtureProjectReferencesPassInOrder(t *testing.T) {
	needCompiler(t)
	checkSolutions(t, tally(t))
}

func run(t *testing.T, e *Engine, l *Lesson, mode Mode) *Result {
	t.Helper()
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
	defer cancel()
	r, err := e.Run(ctx, l, mode)
	if err != nil {
		t.Fatal(err)
	}
	r.Output = stripANSI(r.Output)
	return r
}

func TestProjectContractNamesWhatDoesntMatch(t *testing.T) {
	needCompiler(t)
	c := tally(t)
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	words, lines := c.Lesson("tally-01-words"), c.Lesson("tally-02-lines")

	r := run(t, e, words, ModeRun)
	if r.Stage != "contract" || r.Summary != "✗ create tally.hpp" || !strings.Contains(r.Output, "int count_words(const std::string&);") {
		t.Fatalf("an empty project: %s\n%s", r.Summary, r.Output)
	}
	if _, err := os.Stat(filepath.Join(e.WorkDir(words), "words.txt")); err != nil {
		t.Fatal("the first lesson's starter data file should be in the project folder")
	}

	// Lesson 1's reference, with count_lines declared with the wrong type
	// and Stats and stats not there at all.
	ref, _ := words.Solution()
	ref["tally.hpp"] = append(ref["tally.hpp"], "int count_lines(std::string text);\n"...)
	writeFiles(t, e.WorkDir(lines), ref)
	r = run(t, e, lines, ModeRun)
	if r.Stage != "contract" || r.Summary != "✗ tally.hpp doesn't match the lesson's contract" {
		t.Fatalf("summary %q, stage %q\n%s", r.Summary, r.Stage, r.Output)
	}
	for _, want := range []string{
		"✓ int count_words(const std::string&);",
		"✗ struct Stats { … };\n    not declared in tally.hpp",
		"✗ int count_lines(const std::string&);\n    tally.hpp declares it differently:",
		"✗ Stats stats(const std::string&);",
	} {
		if !strings.Contains(r.Output, want) {
			t.Errorf("output lacks %q:\n%s", want, r.Output)
		}
	}
}

// A blank project has no main() until you write one; the checks need the
// program, so Run says that plainly instead of showing the linker.
func TestProjectWithoutMainSaysSo(t *testing.T) {
	needCompiler(t)
	c := tally(t)
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	words := c.Lesson("tally-01-words")
	ref, _ := words.Solution()
	delete(ref, "main.cpp")
	writeFiles(t, e.WorkDir(words), ref)
	r := run(t, e, words, ModeRun)
	if r.Summary != "✗ your project has no main() yet" || strings.Contains(r.Output, "undefined") {
		t.Fatalf("summary %q\n%s", r.Summary, r.Output)
	}

	// Anything else missing at link time still shows the linker's output.
	ref["main.cpp"] = []byte("int count_chars(const char*);\nint main() { return count_chars(\"x\"); }\n")
	writeFiles(t, e.WorkDir(words), ref)
	if r := run(t, e, words, ModeRun); r.Summary != "✗ doesn't compile" || !strings.Contains(r.Output, "count_chars") {
		t.Fatalf("summary %q\n%s", r.Summary, r.Output)
	}
}

// Breaking what an earlier lesson asked for passes Run, which only has the
// lesson's own visible tests, and fails Submit, which names that lesson.
func TestProjectSubmitCatchesEarlierLessonRegression(t *testing.T) {
	needCompiler(t)
	c := tally(t)
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	lines := c.Lesson("tally-02-lines")
	ref, _ := lines.Solution()
	// Words split on spaces only: lesson 1's hidden test has tabs.
	ref["count.cpp"] = []byte(`#include "tally.hpp"
int count_words(const std::string& text) {
    int n = 0;
    bool in_word = false;
    for (char c : text) {
        if (c == ' ') in_word = false;
        else if (!in_word) { in_word = true; ++n; }
    }
    return n;
}
`)
	writeFiles(t, e.WorkDir(lines), ref)
	if r := run(t, e, lines, ModeRun); !r.OK {
		t.Fatalf("Run should pass: %s\n%s", r.Summary, r.Output)
	}
	r := run(t, e, lines, ModeSubmit)
	if r.OK || r.Summary != "✗ 1 test failing" {
		t.Fatalf("Submit: %s\n%s", r.Summary, r.Output)
	}
	if !strings.Contains(r.Output, "from an earlier lesson, “Counting words”") || !strings.Contains(r.Output, "tabs and newlines split words") {
		t.Fatalf("the failure should say it's lesson 1's test:\n%s", r.Output)
	}
}

func TestProjectResetGoesBackToTheLessonsStart(t *testing.T) {
	c := tally(t)
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	words, lines := c.Lesson("tally-01-words"), c.Lesson("tally-02-lines")
	if _, err := e.Reset(lines); err == nil {
		t.Fatal("Reset without a snapshot of the previous lesson should fail")
	}
	backup, err := e.UseReference(words)
	if err != nil || backup != "" {
		t.Fatalf("catching up an empty folder: backup %q, %v", backup, err)
	}
	if err := e.SaveSnapshot(words); err != nil {
		t.Fatal(err)
	}
	work := e.WorkDir(lines)
	writeFiles(t, work, map[string][]byte{"tally.hpp": []byte("mine"), "src/new.cpp": []byte("mine"), ".hidden/x": []byte("x")})

	backup, err = e.Reset(lines)
	if err != nil {
		t.Fatal(err)
	}
	if data, _ := os.ReadFile(filepath.Join(backup, "src", "new.cpp")); string(data) != "mine" {
		t.Fatal("the backup should hold your files")
	}
	got, _ := e.WorkFiles(lines)
	ref, _ := words.Solution()
	if len(got) != len(ref) {
		t.Fatalf("after Reset the folder has %v, want lesson 1's reference", got)
	}
	for _, f := range got {
		if data, _ := os.ReadFile(filepath.Join(work, filepath.FromSlash(f))); string(data) != string(ref[f]) {
			t.Errorf("%s isn't as lesson 1 left it", f)
		}
	}
}

func TestValidateCatchesProjectMistakes(t *testing.T) {
	lesson := func(front string) *fstest.MapFile {
		return &fstest.MapFile{Data: []byte("---\n" + front + "---\nBody.\n")}
	}
	fsys := fstest.MapFS{
		"course.yaml": {Data: []byte(`title: t
phases:
  - id: p
    title: p
    chapters:
      - id: c
        title: c
        lessons:
          - {id: one, title: One}
          - {id: two, title: Two}
          - {id: old, title: Old}
`)},
		"lessons/one/lesson.md": lesson(`project: proj
contract:
  header: proj.hpp
  functions:
    - {name: f, sig: "int"}
retires:
  - {lesson: two}
`),
		"lessons/one/solution/proj.hpp":      {Data: []byte("int f();\n")},
		"lessons/one/tests/visible_test.cpp": {Data: []byte(`TEST_CASE("adds up") {}` + "\n")},
		"lessons/two/lesson.md":              lesson("project: proj\nbase: old\ncontract:\n  header: other.hpp\n  drops: [g]\nretires:\n  - {lesson: one, tests: [nope.cpp], cases: [\"subtracts*\"], checks: [missing]}\n"),
		"lessons/two/starter/data.txt":       {Data: []byte("x")},
		"lessons/old/lesson.md":              lesson("files: [a.cpp]\n"),
		"lessons/old/starter/a.cpp":          {Data: []byte("a")},
		"lessons/old/solution/a.cpp":         {Data: []byte("b")},
	}
	c, err := LoadFS(fsys)
	if err != nil {
		t.Fatal(err)
	}
	err = c.Validate()
	if err == nil {
		t.Fatal("Validate passed")
	}
	for _, want := range []string{
		`one: contract function f: sig "int" isn't a function type`,
		"one: retires two, which isn't an earlier lesson of project proj",
		"two: base and project can't both be set",
		"two: contract header other.hpp isn't the project's header proj.hpp",
		"two: only a project's first lesson can have a starter/",
		"two: contract drops g, which no earlier lesson added",
		"two: retires one's tests/nope.cpp, which it doesn't have",
		`two: retires test cases "subtracts*", which match none of one's`,
		`two: retires check "missing", which one's checks.yaml doesn't have`,
	} {
		if !strings.Contains(err.Error(), want) {
			t.Errorf("Validate doesn't say %q:\n%v", want, err)
		}
	}
}
