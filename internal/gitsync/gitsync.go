// Package gitsync commits the data directory and syncs it with its remote.
package gitsync

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

// ErrBusy means another sysroad process on this machine is syncing.
var ErrBusy = errors.New("sync already in progress")

func git(ctx context.Context, dir string, args ...string) (string, error) {
	cmd := exec.CommandContext(ctx, "git", append([]string{"-C", dir}, args...)...)
	cmd.Env = append(os.Environ(), "GIT_TERMINAL_PROMPT=0", "GIT_MERGE_AUTOEDIT=no")
	// Never let ssh prompt (passphrase, host key) while a TUI owns the terminal.
	if os.Getenv("GIT_SSH_COMMAND") == "" {
		cmd.Env = append(cmd.Env, "GIT_SSH_COMMAND=ssh -o BatchMode=yes")
	}
	var out bytes.Buffer
	cmd.Stdout, cmd.Stderr = &out, &out
	err := cmd.Run()
	s := strings.TrimSpace(out.String())
	if err != nil {
		if s == "" {
			s = err.Error()
		}
		return s, gitError(args[0], s)
	}
	return s, nil
}

// ErrAuth means git has no usable credentials for the remote.
var ErrAuth = errors.New("git couldn't authenticate. With HTTPS URLs run `gh auth setup-git`; with SSH (git@github.com:…) make sure your key is loaded in ssh-agent")

func gitError(sub, out string) error {
	for _, sign := range []string{"could not read Username", "Authentication failed", "Permission denied (publickey)", "terminal prompts disabled"} {
		if strings.Contains(out, sign) {
			return fmt.Errorf("git %s: %w", sub, ErrAuth)
		}
	}
	return fmt.Errorf("git %s: %s", sub, lastLine(out))
}

func lastLine(s string) string {
	lines := strings.Split(strings.TrimSpace(s), "\n")
	return lines[len(lines)-1]
}

func IsRepo(dir string) bool {
	_, err := os.Stat(filepath.Join(dir, ".git"))
	return err == nil
}

func HasRemote(ctx context.Context, dir string) bool {
	out, err := git(ctx, dir, "remote")
	return err == nil && strings.TrimSpace(out) != ""
}

// lock uses an O_EXCL file in .git; stale locks (crashed process) expire.
func lock(dir string) (func(), error) {
	path := filepath.Join(dir, ".git", "sysroad-sync.lock")
	for attempt := 0; attempt < 2; attempt++ {
		f, err := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0o644)
		if err == nil {
			f.Close()
			return func() { os.Remove(path) }, nil
		}
		if st, serr := os.Stat(path); serr == nil && time.Since(st.ModTime()) > 2*time.Minute {
			os.Remove(path)
			continue
		}
		return nil, ErrBusy
	}
	return nil, ErrBusy
}

// Commit stages everything and commits if there are changes.
func Commit(ctx context.Context, dir, msg string) (bool, error) {
	if _, err := git(ctx, dir, "add", "-A"); err != nil {
		return false, err
	}
	if _, err := git(ctx, dir, "diff", "--cached", "--quiet"); err == nil {
		return false, nil
	}
	_, err := git(ctx, dir, "commit", "-q", "-m", msg)
	return err == nil, err
}

// Sync commits local changes, rebases onto the remote and pushes.
// events.jsonl uses merge=union, so concurrent appends never conflict.
func Sync(ctx context.Context, dir string) error {
	unlock, err := lock(dir)
	if err != nil {
		return err
	}
	defer unlock()

	host, _ := os.Hostname()
	if _, err := Commit(ctx, dir, "sysroad: "+strings.Split(host, ".")[0]+" "+time.Now().Format("2006-01-02 15:04")); err != nil {
		return err
	}
	if !HasRemote(ctx, dir) {
		return nil
	}
	branch, err := git(ctx, dir, "symbolic-ref", "--short", "HEAD")
	if err != nil {
		return err
	}
	if _, err := git(ctx, dir, "fetch", "-q", "origin"); err != nil {
		return err
	}
	// Fetch + rebase rather than pull, so it works before an upstream is set
	// and against an empty remote.
	upstream := "origin/" + branch
	if _, err := git(ctx, dir, "rev-parse", "--verify", "-q", upstream); err == nil {
		if _, err := git(ctx, dir, "rebase", "-q", upstream); err != nil {
			git(context.Background(), dir, "rebase", "--abort")
			return fmt.Errorf("can't rebase onto %s (conflicting roadmap edits?) — resolve by hand in %s", upstream, dir)
		}
	}
	_, err = git(ctx, dir, "push", "-q", "-u", "origin", "HEAD")
	return err
}

// Init creates a fresh repo in dir and makes the first commit.
func Init(ctx context.Context, dir, remote string) error {
	if _, err := git(ctx, dir, "init", "-q", "-b", "main"); err != nil {
		return err
	}
	if _, err := Commit(ctx, dir, "sysroad: initial roadmap"); err != nil {
		return err
	}
	if remote != "" {
		if _, err := git(ctx, dir, "remote", "add", "origin", remote); err != nil {
			return err
		}
		return Sync(ctx, dir)
	}
	return nil
}

func Clone(ctx context.Context, remote, dir string) error {
	cmd := exec.CommandContext(ctx, "git", "clone", "-q", remote, dir)
	cmd.Env = append(os.Environ(), "GIT_TERMINAL_PROMPT=0")
	out, err := cmd.CombinedOutput()
	if err != nil {
		return gitError("clone", string(out))
	}
	return nil
}
