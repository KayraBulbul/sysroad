package gui

import (
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

// UseLoginPath gives the app the PATH your login shell has. On macOS an app
// started from the Dock or Spotlight gets a bare PATH (/usr/bin:/bin:…), so
// it wouldn't find Homebrew's clang-format or anything else you installed.
// Linux launchers pass the session's environment on, so it's a no-op there.
func UseLoginPath() {
	if runtime.GOOS != "darwin" {
		return
	}
	dirs := strings.Split(os.Getenv("PATH"), string(os.PathListSeparator))
	shell := os.Getenv("SHELL")
	if shell == "" {
		shell = "/bin/zsh"
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	if out, err := exec.CommandContext(ctx, shell, "-l", "-c", `printf %s "$PATH"`).Output(); err == nil {
		dirs = append(strings.Split(strings.TrimSpace(string(out)), ":"), dirs...)
	}
	home, _ := os.UserHomeDir()
	dirs = append(dirs, "/opt/homebrew/bin", "/usr/local/bin", filepath.Join(home, "go", "bin"))
	os.Setenv("PATH", joinPath(dirs))
}

// joinPath makes a PATH value of dirs, dropping empty and repeated entries.
func joinPath(dirs []string) string {
	seen := map[string]bool{}
	var path []string
	for _, d := range dirs {
		if d != "" && !seen[d] {
			seen[d] = true
			path = append(path, d)
		}
	}
	return strings.Join(path, string(os.PathListSeparator))
}
