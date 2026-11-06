package app

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/roadmap"
	"github.com/KayraBulbul/sysroad/internal/store"
)

func open(t *testing.T) *App {
	t.Helper()
	dir := t.TempDir()
	if err := Seed(dir); err != nil {
		t.Fatal(err)
	}
	a, err := Open(dir)
	if err != nil {
		t.Fatal(err)
	}
	return a
}

func TestCourseIsTheRoadmap(t *testing.T) {
	a := open(t)
	if len(a.Roadmap.Sections) < 10 {
		t.Fatalf("only %d sections", len(a.Roadmap.Sections))
	}
	for _, s := range a.Roadmap.Sections {
		if !s.ReadOnly {
			t.Errorf("course section %s should be read-only", s.ID)
		}
	}
	if err := a.Course.Validate(); err != nil {
		t.Fatal(err)
	}
}

func TestCurrentAndUpcoming(t *testing.T) {
	a := open(t)
	first := a.Roadmap.Sections[0].Steps[0]
	cur, ok := a.Current()
	if !ok || cur.Step.ID != first.ID {
		t.Fatalf("current = %v, want first step", cur.Step)
	}
	if err := a.SetStatus(first.ID, store.Done); err != nil {
		t.Fatal(err)
	}
	second := a.Roadmap.Sections[0].Steps[1]
	if cur, _ := a.Current(); cur.Step.ID != second.ID {
		t.Fatalf("current after done = %s", cur.Step.ID)
	}
	later := a.Roadmap.Sections[1].Steps[2]
	if err := a.SetStatus(later.ID, store.Doing); err != nil {
		t.Fatal(err)
	}
	if cur, _ := a.Current(); cur.Step.ID != later.ID {
		t.Fatalf("current = %s, want doing step %s", cur.Step.ID, later.ID)
	}
	if done := a.RecentDone(5); len(done) != 1 || done[0].Step.ID != first.ID {
		t.Fatalf("recent done = %+v", done)
	}
}

func TestXPAndLevels(t *testing.T) {
	a := open(t)
	if a.XP() != 0 {
		t.Fatalf("xp = %d", a.XP())
	}
	l := a.Lesson("cpp-01-echo")
	a.SetStatus(l.ID, store.Done)
	if a.XP() != course.XPFor(l) {
		t.Fatalf("xp = %d, want %d", a.XP(), course.XPFor(l))
	}
	for xp, want := range map[int]int{0: 1, 24: 1, 25: 2, 99: 2, 100: 3, 225: 4} {
		if lvl, _, _ := course.Level(xp); lvl != want {
			t.Errorf("Level(%d) = %d, want %d", xp, lvl, want)
		}
	}
}

func TestOwnSectionsRoundTrip(t *testing.T) {
	a := open(t)
	sec := &roadmap.Section{ID: roadmap.NewID("", "Mine"), Title: "Mine"}
	sec.Steps = []*roadmap.Step{{ID: roadmap.NewID(sec.ID, "My Step!"), Title: "My Step!", Detail: "line 1\nline 2"}}
	a.Roadmap.Sections = append(a.Roadmap.Sections, sec)
	if err := a.SaveRoadmap(); err != nil {
		t.Fatal(err)
	}
	data, _ := os.ReadFile(filepath.Join(a.Dir, CustomFile))
	if strings.Contains(string(data), "cpp-01-echo") {
		t.Fatal("course sections leaked into custom.yaml")
	}
	b, err := Open(a.Dir)
	if err != nil {
		t.Fatal(err)
	}
	got := b.Roadmap.Sections[len(b.Roadmap.Sections)-1]
	if got.Title != "Mine" || got.ReadOnly || got.Steps[0].Detail != "line 1\nline 2" {
		t.Fatalf("round trip: %+v", got)
	}
}

func TestTimerLogsTimeAndStartsStep(t *testing.T) {
	a := open(t)
	id := a.Roadmap.Sections[1].Steps[0].ID
	if err := a.StartTimer(id); err != nil {
		t.Fatal(err)
	}
	if a.Timer == nil || a.Progress.StatusOf(id) != store.Doing {
		t.Fatal("timer should run and step should be doing")
	}
	mins, err := a.StopTimer()
	if err != nil || mins != 1 || a.Timer != nil || a.Progress.Minutes[id] != 1 {
		t.Fatalf("stop: mins=%d err=%v timer=%v logged=%d", mins, err, a.Timer, a.Progress.Minutes[id])
	}
}

func TestMigrateFromCRoadmap(t *testing.T) {
	dir := t.TempDir()
	os.Mkdir(filepath.Join(dir, ".git"), 0o755)
	os.WriteFile(filepath.Join(dir, OldRoadmap), []byte("title: old\nsections: []\n"), 0o644)
	os.WriteFile(filepath.Join(dir, ".gitignore"), []byte(".local/\n*.tmp\n"), 0o644)
	ev := filepath.Join(dir, EventsFile)
	store.Append(ev, store.Event{Type: store.EvStatus, ID: "setup-2", Value: "done"})
	store.Append(ev, store.Event{Type: store.EvStatus, ID: "setup-1", Value: "doing"})
	store.Append(ev, store.Event{Type: store.EvNote, ID: "setup-1", Text: "51 mins into lecture 1"})

	a, err := Open(dir)
	if err != nil {
		t.Fatal(err)
	}
	if exists(filepath.Join(dir, OldRoadmap)) || !exists(filepath.Join(dir, ArchiveDir, "roadmap-c.yaml")) {
		t.Fatal("old roadmap should be archived")
	}
	if a.Progress.StatusOf("setup-toolchain") != store.Done {
		t.Fatal("toolchain step should carry over as done")
	}
	if a.Progress.StatusOf("side-missing-semester") != store.Todo || a.Progress.Notes["side-missing-semester"] == "" {
		t.Fatal("Missing Semester should carry its note but stay todo")
	}
	gi, _ := os.ReadFile(filepath.Join(dir, ".gitignore"))
	if !strings.Contains(string(gi), "build/") {
		t.Fatalf(".gitignore not updated: %q", gi)
	}
	// Opening again changes nothing.
	n := len(a.Progress.Events)
	b, err := Open(dir)
	if err != nil || len(b.Progress.Events) != n {
		t.Fatalf("second open: err=%v events %d -> %d", err, n, len(b.Progress.Events))
	}
}

func TestResolveLessonFromWorkDir(t *testing.T) {
	a := open(t)
	l := a.Lesson("cpp-02-words")
	dir, err := a.Touch(l)
	if err != nil {
		t.Fatal(err)
	}
	got, err := a.ResolveLesson("", dir)
	if err != nil || got.ID != l.ID {
		t.Fatalf("resolve from %s = %v, %v", dir, got, err)
	}
	if a.Progress.StatusOf(l.ID) != store.Doing {
		t.Fatal("touching a lesson should mark it in progress")
	}
}
