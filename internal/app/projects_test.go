package app

import (
	"context"
	"errors"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// openTally opens a fresh data dir on the course package's fixture
// project: tally-01-words, tally-02-lines and tally-03-report.
func openTally(t *testing.T) *App {
	t.Helper()
	c, err := course.LoadFS(os.DirFS("../course/testdata/tally"))
	if err != nil {
		t.Fatal(err)
	}
	dir := t.TempDir()
	if err := Seed(dir); err != nil {
		t.Fatal(err)
	}
	a, err := openCourse(dir, c)
	if err != nil {
		t.Fatal(err)
	}
	return a
}

func TestProjectLessonsGoInStrictOrder(t *testing.T) {
	a := openTally(t)
	words, lines := a.Lesson("tally-01-words"), a.Lesson("tally-02-lines")
	var order *OrderError
	if err := a.BeginRun(lines); !errors.As(err, &order) || !strings.Contains(err.Error(), `"Counting words"`) {
		t.Fatalf("running a locked lesson: %v", err)
	}
	if _, err := a.Touch(lines); err != nil || a.Progress.StatusOf(lines.ID) != store.Todo {
		t.Fatalf("opening a locked lesson shouldn't start it: %v, %s", err, a.Progress.StatusOf(lines.ID))
	}
	if err := a.ToggleStatus(lines.ID, store.Doing); !errors.As(err, &order) {
		t.Fatalf("marking a locked lesson in progress: %v", err)
	}
	if err := a.StartTimer(lines.ID); !errors.As(err, &order) || a.Timer != nil {
		t.Fatalf("timing a locked lesson: %v, timer %v", err, a.Timer)
	}
	if cur, _ := a.Current(); cur.Step.ID == lines.ID {
		t.Fatal("the current step should never be a locked lesson")
	}
	for _, s := range []store.Status{store.Done, store.Skipped} {
		if err := a.ToggleStatus(words.ID, s); !errors.Is(err, ErrProjectStatus) {
			t.Fatalf("marking a project lesson %s by hand: %v", s, err)
		}
	}
	work, err := a.Touch(words)
	if err != nil {
		t.Fatal(err)
	}
	if got, _ := a.ResolveLesson("", work); got != words {
		t.Fatalf("in the project folder: %v", got)
	}
	if got, _ := a.EditFile(words); got != "tally.hpp" {
		t.Fatalf("edit before any code = %q, want the contract header", got)
	}

	// Catching up is the way past a lesson you haven't passed.
	if _, err := a.CatchUp(words); err != nil {
		t.Fatal(err)
	}
	if a.Progress.StatusOf(words.ID) != store.Skipped || a.XP() != 0 {
		t.Fatalf("caught-up lesson: %s, %d XP", a.Progress.StatusOf(words.ID), a.XP())
	}
	ref, _ := words.Solution()
	for name, data := range ref {
		if got, _ := os.ReadFile(filepath.Join(work, name)); string(got) != string(data) {
			t.Errorf("%s isn't the reference after catching up", name)
		}
	}
	if err := a.ToggleStatus(words.ID, store.Skipped); !errors.Is(err, ErrProjectStatus) {
		t.Fatal("un-skipping a project lesson by hand should be refused")
	}
	if err := a.BeginRun(words); !errors.As(err, &order) || !strings.Contains(err.Error(), `"Counting lines"`) {
		t.Fatalf("running a passed lesson: %v", err)
	}
	if got, _ := a.ResolveLesson("", work); got != lines {
		t.Fatalf("the project folder now means %v, want the next lesson", got)
	}
	// The lesson it's on now can be started; a skipped one can still be timed.
	if err := a.ToggleStatus(lines.ID, store.Doing); err != nil || a.Progress.StatusOf(lines.ID) != store.Doing {
		t.Fatalf("marking the current lesson in progress: %v", err)
	}
	if err := a.StartTimer(words.ID); err != nil || a.Progress.StatusOf(words.ID) != store.Skipped {
		t.Fatalf("timing a skipped lesson: %v, %s", err, a.Progress.StatusOf(words.ID))
	}
	if _, err := a.StopTimer(); err != nil {
		t.Fatal(err)
	}

	later := time.Now().Add(time.Minute)
	if err := os.Chtimes(filepath.Join(work, "main.cpp"), later, later); err != nil {
		t.Fatal(err)
	}
	if got, _ := a.EditFile(lines); got != "main.cpp" {
		t.Fatalf("edit = %q, want the source changed last", got)
	}
}

func TestProjectSubmitSnapshotsAndResetGoesBackToIt(t *testing.T) {
	if _, err := exec.LookPath("clang++"); err != nil && os.Getenv("SYSROAD_CXX") == "" {
		t.Skip("no clang++")
	}
	a := openTally(t)
	words, lines, report := a.Lesson("tally-01-words"), a.Lesson("tally-02-lines"), a.Lesson("tally-03-report")
	if _, err := a.CatchUp(words); err != nil {
		t.Fatal(err)
	}
	work := a.Engine.WorkDir(lines)
	ref, _ := lines.Solution()
	for name, data := range ref {
		if err := os.MkdirAll(filepath.Dir(filepath.Join(work, name)), 0o755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(work, name), data, 0o644); err != nil {
			t.Fatal(err)
		}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
	defer cancel()
	r, out, err := a.RunLesson(ctx, lines, course.ModeSubmit)
	if err != nil {
		t.Fatal(err)
	}
	if !r.OK || !out.Completed || a.ProjectCurrent(lines.Project()) != report {
		t.Fatalf("submitting lesson 2's reference: %s, completed %v", r.Summary, out.Completed)
	}

	if err := os.WriteFile(filepath.Join(work, "src", "lines.cpp"), []byte("mine"), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := a.ResetLesson(lines); err == nil {
		t.Fatal("resetting a passed lesson should be refused")
	}
	backup, err := a.ResetLesson(report)
	if err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(filepath.Join(backup, "src", "lines.cpp")); string(got) != "mine" {
		t.Fatal("the backup should hold your files")
	}
	if got, _ := os.ReadFile(filepath.Join(work, "src", "lines.cpp")); string(got) != string(ref["src/lines.cpp"]) {
		t.Fatal("Reset should put back the folder as it was when lesson 2 passed")
	}
}
