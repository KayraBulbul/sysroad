package gui

import (
	"errors"
	"os"
	"os/exec"
	"runtime"
	"strings"
)

// openInTerminal runs an interactive lesson program in a terminal window,
// which waits for Enter when it exits so you can read the output. The GUI
// has no terminal of its own yet.
func openInTerminal(cmd *exec.Cmd, title string) error {
	script := "cd " + shQuote(cmd.Dir) + " && " + envPrefix(cmd.Env) + shJoin(cmd.Args) +
		`; code=$?; printf '\n\033[90m[exit %d] press Enter to close\033[0m' "$code"; read _`
	var c *exec.Cmd
	switch runtime.GOOS {
	case "darwin":
		// Terminal.app is always there; it runs the script in a new window.
		c = exec.Command("osascript", "-e",
			`tell application "Terminal" to do script "`+strings.NewReplacer(`\`, `\\`, `"`, `\"`).Replace(script)+`"`,
			"-e", `tell application "Terminal" to activate`)
	default:
		launcher := ""
		for _, t := range []string{os.Getenv("TERMINAL"), "xdg-terminal-exec", "ghostty", "alacritty", "kitty", "foot"} {
			if t != "" {
				if _, err := exec.LookPath(t); err == nil {
					launcher = t
					break
				}
			}
		}
		if launcher == "" {
			return errors.New("no terminal found — set $TERMINAL")
		}
		args := []string{"-e", "sh", "-c", script}
		if launcher == "xdg-terminal-exec" {
			args = []string{"--title=" + title, "--", "sh", "-c", script}
		}
		c = exec.Command(launcher, args...)
	}
	if err := c.Start(); err != nil {
		return err
	}
	go c.Wait()
	return nil
}

// envPrefix passes on the variables the engine added to the environment
// (the sanitizer options); the rest the terminal has already.
func envPrefix(env []string) string {
	var out []string
	for _, kv := range env {
		k, _, _ := strings.Cut(kv, "=")
		if strings.HasSuffix(k, "SAN_OPTIONS") {
			out = append(out, shQuote(kv))
		}
	}
	if len(out) == 0 {
		return ""
	}
	return "env " + strings.Join(out, " ") + " "
}

func shJoin(args []string) string {
	q := make([]string, len(args))
	for i, a := range args {
		q[i] = shQuote(a)
	}
	return strings.Join(q, " ")
}

func shQuote(s string) string { return "'" + strings.ReplaceAll(s, "'", `'\''`) + "'" }
