package tui

import (
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
	"time"
)

// isolate points every place openBeside looks at into the test, so it never
// finds the developer's own nvim, tmux or herdr.
func isolate(t *testing.T) string {
	t.Helper()
	tmp := t.TempDir()
	for k, v := range map[string]string{
		"XDG_RUNTIME_DIR": tmp, "TMPDIR": tmp, "HERDR_ENV": "", "HERDR_PANE_ID": "",
		"TMUX": "", "TMUX_PANE": "", "WAYLAND_DISPLAY": "", "DISPLAY": "", "VISUAL": "",
	} {
		t.Setenv(k, v)
	}
	return tmp
}

func waitFor(t *testing.T, what string, ok func() bool) {
	t.Helper()
	for end := time.Now().Add(5 * time.Second); time.Now().Before(end); time.Sleep(20 * time.Millisecond) {
		if ok() {
			return
		}
	}
	t.Fatalf("timed out waiting for %s", what)
}

// headlessNvim starts nvim in dir and returns its socket.
func headlessNvim(t *testing.T, dir string) string {
	t.Helper()
	cmd := exec.Command("nvim", "--headless", "--clean")
	cmd.Dir = dir
	if err := cmd.Start(); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { cmd.Process.Kill(); cmd.Wait() })
	sock := filepath.Join(os.Getenv("XDG_RUNTIME_DIR"), "nvim."+strconv.Itoa(cmd.Process.Pid)+".0")
	waitFor(t, "nvim's socket", func() bool { _, err := os.Stat(sock); return err == nil })
	return sock
}

func nvimBuffer(t *testing.T, sock string) string {
	t.Helper()
	out, err := exec.Command("nvim", "--server", sock, "--remote-expr", `expand("%:p")`).Output()
	if err != nil {
		t.Fatal(err)
	}
	return strings.TrimSpace(string(out))
}

func TestSidebarEditOpensInTheNvimWorkingInTheDataRepo(t *testing.T) {
	if _, err := exec.LookPath("nvim"); err != nil {
		t.Skip("no nvim")
	}
	tmp := isolate(t)
	root := filepath.Join(tmp, "data")
	dir := filepath.Join(root, "work", "conch")
	elsewhere := filepath.Join(tmp, "elsewhere")
	for _, d := range []string{dir, elsewhere} {
		os.MkdirAll(d, 0o755)
	}
	file := filepath.Join(dir, "main.cpp")
	os.WriteFile(file, []byte("int main() {}\n"), 0o644)

	other := headlessNvim(t, elsewhere)
	if msg := openBeside(root, dir, file)().(besideMsg); msg.inPlace == nil {
		t.Fatalf("with only an nvim outside the data repo, got %+v, want the editor in place", msg)
	}
	mine := headlessNvim(t, root)

	msg := openBeside(root, dir, file)().(besideMsg)
	if msg.err != nil || msg.inPlace != nil {
		t.Fatalf("got %+v, want the file opened in the running nvim", msg)
	}
	if got := nvimBuffer(t, mine); got != realPath(file) {
		t.Errorf("nvim in the data repo shows %q, want %q", got, file)
	}
	if got := nvimBuffer(t, other); got != "" {
		t.Errorf("an nvim outside the data repo was sent %q", got)
	}
}

func TestSidebarEditOpensATmuxPaneLeftOfTheSidebar(t *testing.T) {
	if _, err := exec.LookPath("tmux"); err != nil {
		t.Skip("no tmux")
	}
	tmp := isolate(t)
	dir := filepath.Join(tmp, "lesson")
	os.MkdirAll(dir, 0o755)
	file := filepath.Join(dir, "main.cpp")
	got := filepath.Join(tmp, "editor-ran")
	editor := filepath.Join(tmp, "editor")
	os.WriteFile(editor, []byte("#!/bin/sh\necho \"$PWD $1\" > "+got+"\nsleep 30\n"), 0o755)
	t.Setenv("EDITOR", editor)

	sock := filepath.Join(tmp, "tmux.sock")
	tmux := func(args ...string) string {
		out, err := exec.Command("tmux", append([]string{"-S", sock}, args...)...).CombinedOutput()
		if err != nil {
			t.Fatalf("tmux %v: %v\n%s", args, err, out)
		}
		return strings.TrimSpace(string(out))
	}
	self := tmux("new-session", "-d", "-x", "200", "-y", "50", "-P", "-F", "#{pane_id}", "sleep 30")
	t.Cleanup(func() { exec.Command("tmux", "-S", sock, "kill-server").Run() })
	t.Setenv("TMUX", sock+",0,0")
	t.Setenv("TMUX_PANE", self)

	msg := openBeside(filepath.Join(tmp, "data"), dir, file)().(besideMsg)
	if msg.err != nil || msg.inPlace != nil {
		t.Fatalf("got %+v, want a new tmux pane", msg)
	}
	waitFor(t, "the editor to start", func() bool { _, err := os.Stat(got); return err == nil })
	b, _ := os.ReadFile(got)
	if want := realPath(dir) + " " + file; strings.TrimSpace(string(b)) != want {
		t.Errorf("editor ran as %q, want %q", strings.TrimSpace(string(b)), want)
	}
	if tmux("display-message", "-p", "-t", self, "#{pane_at_right}") != "1" {
		t.Error("the sidebar is no longer the rightmost pane")
	}
}
