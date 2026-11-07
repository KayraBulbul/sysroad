package gui

import (
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func git(t *testing.T, dir string, args ...string) {
	t.Helper()
	cmd := exec.Command("git", args...)
	cmd.Dir = dir
	cmd.Env = append(os.Environ(), "GIT_AUTHOR_NAME=t", "GIT_AUTHOR_EMAIL=t@t", "GIT_COMMITTER_NAME=t", "GIT_COMMITTER_EMAIL=t@t")
	if out, err := cmd.CombinedOutput(); err != nil {
		t.Fatalf("git %v: %v\n%s", args, err, out)
	}
}

// The updater sees new commits on its branch, reports them, and on Update
// moves its clone there and runs `make install-app`; a failing build is
// reported, not passed off as ready.
func TestUpdaterFollowsBranch(t *testing.T) {
	if _, err := exec.LookPath("make"); err != nil {
		t.Skip("make not installed")
	}
	root := t.TempDir()
	work, src := filepath.Join(root, "work"), filepath.Join(root, "src")
	commit := func(file, makefile, msg string) {
		os.WriteFile(filepath.Join(work, "Makefile"), []byte(makefile), 0o644)
		os.WriteFile(filepath.Join(work, file), []byte(msg), 0o644)
		git(t, work, "add", "-A")
		git(t, work, "commit", "-q", "-m", msg)
	}
	os.MkdirAll(work, 0o755)
	git(t, root, "init", "-q", "-b", "main", work)
	commit("a", "install-app:\n\ttouch installed\n", "first")
	git(t, root, "clone", "-q", work, src)

	u := newUpdater(src, func(UpdateInfo) {})
	if !u.info.Managed || u.info.Branch != "main" {
		t.Fatalf("clone not recognised: %+v", u.info)
	}
	if err := u.check(); err != nil || u.snapshot().Behind != 0 {
		t.Fatalf("fresh clone: %+v %v", u.snapshot(), err)
	}

	commit("b", "install-app:\n\ttouch installed\n", "new lessons")
	if err := u.check(); err != nil {
		t.Fatal(err)
	}
	if i := u.snapshot(); i.Behind != 1 || len(i.Commits) != 1 || i.Commits[0] != "new lessons" {
		t.Fatalf("after a new commit: %+v", i)
	}

	wait := func() UpdateInfo {
		deadline := time.Now().Add(10 * time.Second)
		for time.Now().Before(deadline) {
			if i := u.snapshot(); i.State != "building" {
				return i
			}
			time.Sleep(20 * time.Millisecond)
		}
		t.Fatal("build never finished")
		return UpdateInfo{}
	}
	if err := u.update(); err != nil {
		t.Fatal(err)
	}
	if i := wait(); i.State != "ready" {
		t.Fatalf("update: %+v", i)
	}
	if _, err := os.Stat(filepath.Join(src, "b")); err != nil {
		t.Fatal("clone wasn't moved to the new commit")
	}
	if _, err := os.Stat(filepath.Join(src, "installed")); err != nil {
		t.Fatal("make install-app didn't run")
	}

	// A broken build is reported as failed with its output.
	u.info.State = "idle"
	commit("c", "install-app:\n\t@echo compile error here; false\n", "broken")
	if err := u.update(); err != nil {
		t.Fatal(err)
	}
	i := wait()
	if i.State != "failed" || i.Error == "" || !strings.Contains(strings.Join(i.Log, "\n"), "compile error here") {
		t.Fatalf("broken build: %+v", i)
	}
}
