package tui

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"os/user"
	"path/filepath"
	"runtime"
	"strings"
	"syscall"
	"time"

	tea "github.com/charmbracelet/bubbletea"
)

// besideMsg reports where openBeside put the file. inPlace is set when there
// was nowhere else to open it, and the sidebar runs it itself.
type besideMsg struct {
	where   string
	inPlace *exec.Cmd
	err     error
}

// openBeside opens file in an editor outside the sidebar's own pane, so the
// lesson stays on screen: an nvim already working in the data repo (root),
// else a new herdr or tmux pane, else a new terminal window. When none of
// those is available the editor runs in place, as in full-screen mode.
func openBeside(root, dir, file string) tea.Cmd {
	return func() tea.Msg {
		if sock := nvimIn(root, file); sock != "" {
			err := exec.Command("nvim", "--server", sock, "--remote", file).Run()
			return besideMsg{where: "your nvim", err: err}
		}
		shell := `exec ` + EditorCommand() + ` "$1"`
		if os.Getenv("HERDR_ENV") == "1" && os.Getenv("HERDR_PANE_ID") != "" {
			return besideMsg{where: "a new pane", err: herdrOpen(dir, shell, file)}
		}
		if os.Getenv("TMUX") != "" && os.Getenv("TMUX_PANE") != "" {
			return besideMsg{where: "a new pane", err: tmuxOpen(dir, shell, file)}
		}
		if cmd := terminalWindow(shell, file); cmd != nil {
			cmd.Dir = dir
			cmd.SysProcAttr = &syscall.SysProcAttr{Setsid: true} // outlive the sidebar
			if err := cmd.Start(); err != nil {
				return besideMsg{err: err}
			}
			go cmd.Wait()
			return besideMsg{where: "a new window"}
		}
		cmd := exec.Command("sh", "-c", shell, "sh", file)
		cmd.Dir = dir
		return besideMsg{inPlace: cmd}
	}
}

// nvimIn finds the server socket of a running nvim whose working directory is
// inside root, preferring the one whose directory holds file. An nvim working
// anywhere else is someone else's and is left alone.
func nvimIn(root, file string) string {
	if _, err := exec.LookPath("nvim"); err != nil {
		return ""
	}
	root = realPath(root)
	file = realPath(file)
	var socks []string
	if d := os.Getenv("XDG_RUNTIME_DIR"); d != "" {
		socks, _ = filepath.Glob(filepath.Join(d, "nvim.*.0"))
	}
	if u, err := user.Current(); err == nil { // nvim's fallback, and the Mac's
		more, _ := filepath.Glob(filepath.Join(os.TempDir(), "nvim."+u.Username, "*", "nvim.*.0"))
		socks = append(socks, more...)
	}
	best, bestLen := "", -1
	for _, s := range socks {
		ctx, cancel := context.WithTimeout(context.Background(), time.Second)
		out, err := exec.CommandContext(ctx, "nvim", "--server", s, "--remote-expr", "getcwd()").Output()
		cancel()
		if err != nil {
			continue // stale socket or a busy nvim
		}
		cwd := realPath(strings.TrimSpace(string(out)))
		if !within(cwd, root) {
			continue
		}
		n := 0
		if within(file, cwd) {
			n = len(cwd)
		}
		if n > bestLen {
			best, bestLen = s, n
		}
	}
	return best
}

func realPath(p string) string {
	if r, err := filepath.EvalSymlinks(p); err == nil {
		return r
	}
	return filepath.Clean(p)
}

func within(p, dir string) bool {
	return p == dir || strings.HasPrefix(p, dir+string(filepath.Separator))
}

// splitTarget picks the pane to split: the widest one other than the
// sidebar's, which is usually the editor-side split; the sidebar itself when
// it is alone.
func splitTarget(self string, panes map[string]int) string {
	target, w := self, -1
	for id, width := range panes {
		if id != self && width > w {
			target, w = id, width
		}
	}
	return target
}

// herdrOpen splits a herdr pane next to the sidebar and starts the editor in it.
func herdrOpen(dir, shell, file string) error {
	self := os.Getenv("HERDR_PANE_ID")
	var layout struct {
		Result struct {
			Layout struct {
				Panes []struct {
					ID   string `json:"pane_id"`
					Rect struct{ Width int }
				}
			}
		}
	}
	out, err := exec.Command("herdr", "pane", "layout", "--pane", self).Output()
	if err != nil {
		return err
	}
	if err := json.Unmarshal(out, &layout); err != nil {
		return err
	}
	panes := map[string]int{}
	for _, p := range layout.Result.Layout.Panes {
		panes[p.ID] = p.Rect.Width
	}
	target := splitTarget(self, panes)
	var split struct {
		Result struct {
			Pane struct {
				ID string `json:"pane_id"`
			}
		}
	}
	out, err = exec.Command("herdr", "pane", "split", target, "--direction", "right", "--cwd", dir, "--focus").Output()
	if err != nil {
		return err
	}
	if err := json.Unmarshal(out, &split); err != nil {
		return err
	}
	pane := split.Result.Pane.ID
	if pane == "" {
		return errors.New("herdr didn't say which pane it made")
	}
	if target == self { // keep the sidebar on the right
		exec.Command("herdr", "pane", "swap", "--source-pane", pane, "--target-pane", self).Run()
	}
	// The new pane runs the user's shell, so type a command that replaces it:
	// the pane closes when the editor does.
	return exec.Command("herdr", "pane", "run", pane, "exec sh -c "+shq(shell)+" sh "+shq(file)).Run()
}

// tmuxOpen splits a tmux pane next to the sidebar and starts the editor in it.
func tmuxOpen(dir, shell, file string) error {
	self := os.Getenv("TMUX_PANE")
	out, err := exec.Command("tmux", "list-panes", "-t", self, "-F", "#{pane_id} #{pane_width}").Output()
	if err != nil {
		return err
	}
	panes := map[string]int{}
	for _, line := range strings.Split(strings.TrimSpace(string(out)), "\n") {
		var id string
		var w int
		if _, err := fmt.Sscan(line, &id, &w); err == nil {
			panes[id] = w
		}
	}
	target := splitTarget(self, panes)
	args := []string{"split-window", "-h", "-t", target, "-c", dir}
	if target == self {
		args = append(args, "-b") // keep the sidebar on the right
	}
	args = append(args, "sh", "-c", shell, "sh", file)
	return exec.Command("tmux", args...).Run()
}

// terminalWindow is a command that runs the editor in a new terminal window,
// or nil when there is no desktop to open one on.
func terminalWindow(shell, file string) *exec.Cmd {
	if runtime.GOOS != "linux" || (os.Getenv("WAYLAND_DISPLAY") == "" && os.Getenv("DISPLAY") == "") {
		return nil
	}
	// Omarchy's launcher knows whether its editor needs a terminal.
	if strings.HasPrefix(EditorCommand(), "omarchy-launch-editor") {
		if _, err := exec.LookPath("omarchy-launch-editor"); err == nil {
			return exec.Command("omarchy-launch-editor", file)
		}
	}
	if _, err := exec.LookPath("xdg-terminal-exec"); err == nil {
		return exec.Command("xdg-terminal-exec", "sh", "-c", shell, "sh", file)
	}
	if t := os.Getenv("TERMINAL"); t != "" {
		if _, err := exec.LookPath(t); err == nil {
			return exec.Command(t, "-e", "sh", "-c", shell, "sh", file)
		}
	}
	return nil
}

// shq quotes s for a POSIX shell.
func shq(s string) string { return "'" + strings.ReplaceAll(s, "'", `'\''`) + "'" }
