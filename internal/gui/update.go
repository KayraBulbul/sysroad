package gui

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"runtime/debug"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"
)

// EvUpdate carries UpdateInfo whenever the update state changes.
const EvUpdate = "update"

// The app updates itself from its own clone of the sysroad repo (made by
// `make app`): fetch, and if the branch moved on, reset the clone to it and
// run `make install-app`, which rebuilds and reinstalls both binaries. The
// course is built into the binary, so this is also how new lessons arrive.

const (
	updateEvery  = 6 * time.Hour
	firstCheck   = 15 * time.Second
	logLines     = 200
	buildTimeout = 15 * time.Minute
)

// UpdateInfo is the update state the top bar shows.
type UpdateInfo struct {
	Managed bool      `json:"managed"` // installed with `make app`, so it can update itself
	Src     string    `json:"src"`
	Branch  string    `json:"branch"`
	Running string    `json:"running"` // short commit this binary was built from
	Behind  int       `json:"behind"`  // commits on the branch it doesn't have
	Commits []string  `json:"commits"` // their subjects, newest first
	Checked time.Time `json:"checked"`
	State   string    `json:"state"` // idle, checking, building, ready, failed
	Error   string    `json:"error"`
	Log     []string  `json:"log"` // the build's last lines
}

type updater struct {
	src  string
	emit func(UpdateInfo)

	mu   sync.Mutex
	info UpdateInfo
}

// UpdateSrc is the app's clone: $SYSROAD_SRC, else ~/.local/share/sysroad-src.
// It sits next to the data folder, not in it: the data folder is a synced
// git repo of its own.
func UpdateSrc() string {
	if d := os.Getenv("SYSROAD_SRC"); d != "" {
		return d
	}
	home, _ := os.UserHomeDir()
	return filepath.Join(home, ".local", "share", "sysroad-src")
}

func newUpdater(src string, emit func(UpdateInfo)) *updater {
	u := &updater{src: src, emit: emit}
	u.info = UpdateInfo{Src: src, State: "idle", Running: runningCommit()}
	if isDir(filepath.Join(src, ".git")) {
		u.info.Managed = true
		u.info.Branch, _ = gitOut(context.Background(), src, "rev-parse", "--abbrev-ref", "HEAD")
	}
	return u
}

func isDir(p string) bool { st, err := os.Stat(p); return err == nil && st.IsDir() }

// runningCommit is the commit this binary was built from, which Go records
// when building inside a git checkout.
func runningCommit() string {
	bi, ok := debug.ReadBuildInfo()
	if !ok {
		return ""
	}
	for _, s := range bi.Settings {
		if s.Key == "vcs.revision" {
			return s.Value
		}
	}
	return ""
}

func gitOut(ctx context.Context, dir string, args ...string) (string, error) {
	cmd := exec.CommandContext(ctx, "git", args...)
	cmd.Dir = dir
	cmd.Env = append(buildEnv(), "GIT_TERMINAL_PROMPT=0")
	out, err := cmd.CombinedOutput()
	s := strings.TrimSpace(string(out))
	if err != nil {
		if s == "" {
			s = err.Error()
		}
		return "", errors.New(lastLine(s))
	}
	return s, nil
}

func lastLine(s string) string {
	lines := strings.Split(strings.TrimSpace(s), "\n")
	return lines[len(lines)-1]
}

func (u *updater) snapshot() UpdateInfo {
	u.mu.Lock()
	defer u.mu.Unlock()
	i := u.info
	i.Commits = append([]string(nil), i.Commits...)
	i.Log = append([]string(nil), i.Log...)
	return i
}

func (u *updater) set(f func(*UpdateInfo)) {
	u.mu.Lock()
	f(&u.info)
	u.mu.Unlock()
	u.emit(u.snapshot())
}

// check fetches the branch and counts the commits the running binary lacks.
func (u *updater) check() error {
	u.mu.Lock()
	// Nothing to look for while building, or once a build waits for a restart.
	if !u.info.Managed || u.info.State == "building" || u.info.State == "checking" || u.info.State == "ready" {
		u.mu.Unlock()
		return nil
	}
	branch := u.info.Branch
	u.info.State, u.info.Error = "checking", ""
	u.mu.Unlock()
	u.emit(u.snapshot())

	ctx, cancel := context.WithTimeout(context.Background(), 45*time.Second)
	defer cancel()
	behind, commits, err := u.compare(ctx, branch)
	u.set(func(i *UpdateInfo) {
		i.Checked = time.Now()
		i.State = "idle"
		if err != nil {
			i.Error = "couldn't check for updates: " + err.Error()
			return
		}
		i.Behind, i.Commits = behind, commits
	})
	return err
}

func (u *updater) compare(ctx context.Context, branch string) (int, []string, error) {
	if _, err := gitOut(ctx, u.src, "fetch", "--quiet", "origin", branch); err != nil {
		return 0, nil, err
	}
	base := "HEAD"
	if rev := runningCommit(); rev != "" {
		if _, err := gitOut(ctx, u.src, "cat-file", "-e", rev+"^{commit}"); err == nil {
			base = rev
		}
	}
	rng := base + "..origin/" + branch
	n, err := gitOut(ctx, u.src, "rev-list", "--count", rng)
	if err != nil {
		return 0, nil, err
	}
	behind, _ := strconv.Atoi(n)
	var commits []string
	if behind > 0 {
		out, err := gitOut(ctx, u.src, "log", "--format=%s", "-n", "20", rng)
		if err == nil && out != "" {
			commits = strings.Split(out, "\n")
		}
	}
	return behind, commits, nil
}

// update moves the clone to the branch's tip and rebuilds. It returns at
// once; progress arrives as EvUpdate.
func (u *updater) update() error {
	u.mu.Lock()
	if !u.info.Managed {
		u.mu.Unlock()
		return errors.New("this copy wasn't installed with `make app`, so it can't update itself")
	}
	if u.info.State == "building" {
		u.mu.Unlock()
		return nil
	}
	branch := u.info.Branch
	u.info.State, u.info.Error, u.info.Log = "building", "", []string{}
	u.mu.Unlock()
	u.emit(u.snapshot())

	go func() {
		err := u.build(branch)
		u.set(func(i *UpdateInfo) {
			if err != nil {
				i.State, i.Error = "failed", err.Error()
				return
			}
			i.State = "ready"
		})
	}()
	return nil
}

func (u *updater) build(branch string) error {
	ctx, cancel := context.WithTimeout(context.Background(), buildTimeout)
	defer cancel()
	// The clone belongs to the app, so it simply follows the branch.
	for _, args := range [][]string{
		{"fetch", "--quiet", "origin", branch},
		{"reset", "--hard", "--quiet", "origin/" + branch},
	} {
		if _, err := gitOut(ctx, u.src, args...); err != nil {
			return fmt.Errorf("git %s: %w", args[0], err)
		}
	}
	cmd := exec.CommandContext(ctx, "make", "install-app")
	cmd.Dir = u.src
	cmd.Env = buildEnv()
	pr, pw := io.Pipe()
	cmd.Stdout, cmd.Stderr = pw, pw
	done := make(chan struct{})
	go func() {
		defer close(done)
		last := time.Time{}
		sc := bufio.NewScanner(pr)
		sc.Buffer(make([]byte, 64<<10), 1<<20)
		for sc.Scan() {
			line := sc.Text()
			u.mu.Lock()
			u.info.Log = append(u.info.Log, line)
			if len(u.info.Log) > logLines {
				u.info.Log = u.info.Log[len(u.info.Log)-logLines:]
			}
			u.mu.Unlock()
			if time.Since(last) > 150*time.Millisecond {
				last = time.Now()
				u.emit(u.snapshot())
			}
		}
	}()
	err := cmd.Run()
	pw.Close()
	<-done
	if err != nil {
		return fmt.Errorf("the build failed (%v); nothing was changed", err)
	}
	return nil
}

// buildEnv is this process's environment with a PATH that finds go, bun and
// git even when the app was started from a launcher or the Dock.
func buildEnv() []string {
	home, _ := os.UserHomeDir()
	dirs := strings.Split(os.Getenv("PATH"), string(os.PathListSeparator))
	dirs = append(dirs,
		filepath.Join(home, ".local", "share", "mise", "shims"),
		filepath.Join(home, ".local", "bin"),
		filepath.Join(home, ".bun", "bin"),
		filepath.Join(home, "go", "bin"),
		"/opt/homebrew/bin", "/usr/local/go/bin", "/usr/local/bin",
	)
	seen := map[string]bool{}
	var path []string
	for _, d := range dirs {
		if d != "" && !seen[d] {
			seen[d] = true
			path = append(path, d)
		}
	}
	env := []string{"PATH=" + strings.Join(path, string(os.PathListSeparator))}
	for _, kv := range os.Environ() {
		if !strings.HasPrefix(kv, "PATH=") {
			env = append(env, kv)
		}
	}
	return env
}

// relaunch starts the installed app once this process has exited (with a
// single instance, starting it any earlier would just hand back to us).
func relaunch() error {
	home, _ := os.UserHomeDir()
	var launch string
	if runtime.GOOS == "darwin" {
		launch = `open -n "` + filepath.Join(home, "Applications", "sysroad.app") + `"`
	} else {
		exe := filepath.Join(home, ".local", "bin", "sysroad-gui")
		if _, err := os.Stat(exe); err != nil {
			if exe, err = os.Executable(); err != nil {
				return err
			}
		}
		launch = `exec "` + exe + `"`
	}
	script := fmt.Sprintf(`for i in $(seq 100); do kill -0 %d 2>/dev/null || break; sleep 0.1; done; %s`, os.Getpid(), launch)
	cmd := exec.Command("sh", "-c", script)
	cmd.Env = buildEnv()
	cmd.SysProcAttr = &syscall.SysProcAttr{Setsid: true}
	return cmd.Start()
}

// ---- bound methods ----

// UpdateStatus is the current update state.
func (s *Service) UpdateStatus() UpdateInfo { return s.updates.snapshot() }

// CheckUpdates looks for new commits now.
func (s *Service) CheckUpdates() (UpdateInfo, error) {
	err := s.updates.check()
	return s.updates.snapshot(), err
}

// Update rebuilds and reinstalls from the newest commit, in the background.
func (s *Service) Update() error { return s.updates.update() }

// RestartToUpdate quits and starts the freshly installed app.
func (s *Service) RestartToUpdate() error {
	if err := relaunch(); err != nil {
		return err
	}
	go s.quit()
	return nil
}

// checkUpdatesLoop checks shortly after launch and then every few hours.
func (s *Service) checkUpdatesLoop() {
	select {
	case <-s.stop:
		return
	case <-time.After(firstCheck):
	}
	t := time.NewTicker(updateEvery)
	defer t.Stop()
	for {
		s.updates.check()
		select {
		case <-s.stop:
			return
		case <-t.C:
		}
	}
}
