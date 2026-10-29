package gitsync_test

import (
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"testing"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/gitsync"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// Two machines log progress offline, then sync in turn: both histories must
// survive via the union merge on events.jsonl.
func TestTwoMachineSync(t *testing.T) {
	if _, err := exec.LookPath("git"); err != nil {
		t.Skip("git not installed")
	}
	t.Setenv("GIT_AUTHOR_NAME", "test")
	t.Setenv("GIT_AUTHOR_EMAIL", "test@example.com")
	t.Setenv("GIT_COMMITTER_NAME", "test")
	t.Setenv("GIT_COMMITTER_EMAIL", "test@example.com")
	ctx := context.Background()
	root := t.TempDir()
	remote := filepath.Join(root, "remote.git")
	if out, err := exec.Command("git", "init", "-q", "--bare", remote).CombinedOutput(); err != nil {
		t.Fatal(string(out))
	}

	macDir, deskDir := filepath.Join(root, "mac"), filepath.Join(root, "desk")
	if err := app.Seed(macDir); err != nil {
		t.Fatal(err)
	}
	if err := gitsync.Init(ctx, macDir, remote); err != nil {
		t.Fatalf("init with empty remote: %v", err)
	}
	if err := gitsync.Clone(ctx, remote, deskDir); err != nil {
		t.Fatal(err)
	}
	mac, _ := app.Open(macDir)
	desk, err := app.Open(deskDir)
	if err != nil {
		t.Fatal(err)
	}

	// Offline on both: progress events plus a roadmap edit on each side
	// in different places.
	mac.LogTime("c-1", 30)
	mac.SetStatus("c-1", store.Done)
	mac.Roadmap.Sections[0].Title = "00 · Setup (edited on mac)"
	mac.SaveRoadmap()
	desk.LogTime("c-2", 20)
	desk.SetNote("c", "section note from desktop")
	last := desk.Roadmap.Sections[len(desk.Roadmap.Sections)-1]
	last.Title = "Side Quests (edited on desktop)"
	desk.SaveRoadmap()

	for _, dir := range []string{macDir, deskDir, macDir} {
		if err := gitsync.Sync(ctx, dir); err != nil {
			t.Fatalf("sync %s: %v", filepath.Base(dir), err)
		}
	}
	for _, a := range []*app.App{mac, desk} {
		a.Reload(true)
		p := a.Progress
		if p.Minutes["c-1"] != 30 || p.Minutes["c-2"] != 20 || p.StatusOf("c-1") != store.Done || p.Notes["c"] == "" {
			t.Errorf("%s missing merged progress: %+v %+v", filepath.Base(a.Dir), p.Minutes, p.Notes)
		}
		if a.Roadmap.Sections[0].Title != "00 · Setup (edited on mac)" ||
			a.Roadmap.Sections[len(a.Roadmap.Sections)-1].Title != "Side Quests (edited on desktop)" {
			t.Errorf("%s missing merged roadmap edits", filepath.Base(a.Dir))
		}
	}
	if _, err := os.Stat(filepath.Join(macDir, ".git", "sysroad-sync.lock")); err == nil {
		t.Error("sync lock left behind")
	}
}
