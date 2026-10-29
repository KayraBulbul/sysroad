package app

import (
	"testing"

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

func TestDefaultRoadmapIsValid(t *testing.T) {
	r, err := roadmap.Parse(roadmap.DefaultYAML)
	if err != nil {
		t.Fatal(err)
	}
	if len(r.Sections) < 10 {
		t.Fatalf("only %d sections", len(r.Sections))
	}
	for _, s := range r.Sections {
		if len(s.Steps) == 0 {
			t.Errorf("section %s has no steps", s.ID)
		}
	}
}

func TestCurrentAndUpcoming(t *testing.T) {
	a := open(t)
	first := a.Roadmap.Sections[0].Steps[0]
	cur, ok := a.Current()
	if !ok || cur.Step.ID != first.ID {
		t.Fatalf("current = %v, want first step", cur.Step)
	}

	// Completing it advances; an in-progress step elsewhere takes priority.
	if err := a.SetStatus(first.ID, store.Done); err != nil {
		t.Fatal(err)
	}
	if cur, _ := a.Current(); cur.Step.ID != a.Roadmap.Sections[0].Steps[1].ID {
		t.Fatalf("current after done = %s", cur.Step.ID)
	}
	later := a.Roadmap.Sections[3].Steps[2]
	if err := a.SetStatus(later.ID, store.Doing); err != nil {
		t.Fatal(err)
	}
	if cur, _ := a.Current(); cur.Step.ID != later.ID {
		t.Fatalf("current = %s, want doing step %s", cur.Step.ID, later.ID)
	}
	up := a.Upcoming(3)
	if len(up) != 3 || up[0].Step.ID != a.Roadmap.Sections[0].Steps[1].ID {
		t.Fatalf("upcoming = %+v", up)
	}
	if done := a.RecentDone(5); len(done) != 1 || done[0].Step.ID != first.ID {
		t.Fatalf("recent done = %+v", done)
	}
}

func TestOptionalAndSkippedDontCount(t *testing.T) {
	a := open(t)
	sec := a.Roadmap.Section("csapp-data")
	before := a.SectionStats(sec)
	var opt *roadmap.Step
	for _, st := range sec.Steps {
		if st.Optional {
			opt = st
		}
	}
	if opt == nil {
		t.Fatal("expected an optional step in csapp-data")
	}
	if before.Total != len(sec.Steps)-1 {
		t.Fatalf("total = %d, want %d", before.Total, len(sec.Steps)-1)
	}
	a.SetStatus(opt.ID, store.Done)
	a.SetStatus(sec.Steps[0].ID, store.Skipped)
	after := a.SectionStats(sec)
	if after.Total != before.Total || after.Done != 1 {
		t.Fatalf("after = %+v", after)
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

func TestRoadmapEditRoundTrip(t *testing.T) {
	a := open(t)
	sec := a.Roadmap.Sections[0]
	sec.Steps = roadmap.Insert(sec.Steps, 1, &roadmap.Step{ID: roadmap.NewID(sec.ID, "My Step!"), Title: "My Step!"})
	if err := a.SaveRoadmap(); err != nil {
		t.Fatal(err)
	}
	b, err := Open(a.Dir)
	if err != nil {
		t.Fatal(err)
	}
	got := b.Roadmap.Sections[0].Steps[1]
	if got.Title != "My Step!" || got.ID[:len("setup-my-step-")] != "setup-my-step-" {
		t.Fatalf("round trip: %+v", got)
	}
	if b.Roadmap.Sections[0].Steps[0].Detail != a.Roadmap.Sections[0].Steps[0].Detail {
		t.Fatal("multi-line detail changed on save")
	}
}
