package gitsync_test

import (
	"context"
	"errors"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/gitsync"
	"github.com/KayraBulbul/sysroad/internal/roadmap"
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

	// Offline on both: progress events on each side, a section of your own
	// added on the mac, and lesson code written on the desktop.
	mac.LogTime("cpp-01-echo", 30)
	mac.SetStatus("cpp-01-echo", store.Done)
	mac.Roadmap.Sections = append(mac.Roadmap.Sections, &roadmap.Section{ID: "mine", Title: "Mine (from mac)"})
	mac.SaveRoadmap()
	desk.LogTime("cpp-02-words", 20)
	desk.SetNote("cpp-coreutils-wc", "chapter note from desktop")
	work, err := desk.Touch(desk.Lesson("cpp-02-words"))
	if err != nil {
		t.Fatal(err)
	}
	os.WriteFile(filepath.Join(work, "words.cpp"), []byte("// written on the desktop\n"), 0o644)
	os.MkdirAll(filepath.Join(work, "build"), 0o755)
	os.WriteFile(filepath.Join(work, "build", "junk.o"), []byte("x"), 0o644)

	for _, dir := range []string{macDir, deskDir, macDir} {
		if err := gitsync.Sync(ctx, dir); err != nil {
			t.Fatalf("sync %s: %v", filepath.Base(dir), err)
		}
	}
	for _, a := range []*app.App{mac, desk} {
		a.Reload(true)
		p := a.Progress
		if p.Minutes["cpp-01-echo"] != 30 || p.Minutes["cpp-02-words"] != 20 || p.StatusOf("cpp-01-echo") != store.Done || p.Notes["cpp-coreutils-wc"] == "" {
			t.Errorf("%s missing merged progress: %+v %+v", filepath.Base(a.Dir), p.Minutes, p.Notes)
		}
		if a.Roadmap.Sections[len(a.Roadmap.Sections)-1].Title != "Mine (from mac)" {
			t.Errorf("%s missing the section added on the mac", filepath.Base(a.Dir))
		}
	}
	code, err := os.ReadFile(filepath.Join(macDir, "work", "cpp-02-words", "words.cpp"))
	if err != nil || string(code) != "// written on the desktop\n" {
		t.Errorf("lesson code didn't sync to the mac: %q %v", code, err)
	}
	if _, err := os.Stat(filepath.Join(macDir, "work", "cpp-02-words", "build")); err == nil {
		t.Error("build output should be gitignored")
	}
	if _, err := os.Stat(filepath.Join(macDir, ".git", "sysroad-sync.lock")); err == nil {
		t.Error("sync lock left behind")
	}
}

// Starting a lesson on one machine and opening it for the first time on
// the other must pick up the pushed code instead of creating a starter copy
// that conflicts on the next sync.
func TestLessonStartedOnTheOtherMachine(t *testing.T) {
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
	app.Seed(macDir)
	if err := gitsync.Init(ctx, macDir, remote); err != nil {
		t.Fatal(err)
	}
	if err := gitsync.Clone(ctx, remote, deskDir); err != nil {
		t.Fatal(err)
	}
	mac, _ := app.Open(macDir)
	desk, err := app.Open(deskDir)
	if err != nil {
		t.Fatal(err)
	}

	l := mac.Lesson("setup-lessons")
	work, err := mac.Touch(l)
	if err != nil {
		t.Fatal(err)
	}
	os.WriteFile(filepath.Join(work, "add.cpp"), []byte("int add(int a, int b) { return a + b; }\n"), 0o644)
	if err := gitsync.Sync(ctx, macDir); err != nil {
		t.Fatal(err)
	}

	deskWork, err := desk.Touch(desk.Lesson("setup-lessons"))
	if err != nil {
		t.Fatal(err)
	}
	got, _ := os.ReadFile(filepath.Join(deskWork, "add.cpp"))
	if !strings.Contains(string(got), "a + b") {
		t.Fatalf("desktop got a fresh starter instead of the mac's code: %q", got)
	}
	if err := gitsync.Sync(ctx, deskDir); err != nil {
		t.Fatalf("sync after opening the lesson: %v", err)
	}
}

func TestNetworkFailureIsNotReportedAsAuth(t *testing.T) {
	err := gitsync.Clone(context.Background(), "https://user@invalid.invalid/x.git", filepath.Join(t.TempDir(), "x"))
	if err == nil {
		t.Skip("unexpectedly reachable")
	}
	// Not an auth failure: should not claim to be one.
	if errors.Is(err, gitsync.ErrAuth) {
		t.Errorf("DNS failure reported as auth error: %v", err)
	}
}
