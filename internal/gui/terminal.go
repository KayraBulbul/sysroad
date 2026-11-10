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
	script := terminalScript(cmd)
	var c *exec.Cmd
	switch runtime.GOOS {
	case "darwin":
		// Terminal.app runs a .command file in a new window. Unlike asking
		// it over AppleScript, opening a file needs no Automation permission
		// (which an app without NSAppleEventsUsageDescription never gets).
		path, err := commandFile(script)
		if err != nil {
			return err
		}
		c = exec.Command("open", "-a", "Terminal", path)
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

// commandFile writes script to an executable .command file that deletes
// itself when it starts, for Terminal.app to open.
func commandFile(script string) (string, error) {
	f, err := os.CreateTemp("", "sysroad-*.command")
	if err != nil {
		return "", err
	}
	_, werr := f.WriteString("#!/bin/sh\nrm -f \"$0\"\n" + script + "\n")
	cerr := f.Close()
	if err := errors.Join(werr, cerr, os.Chmod(f.Name(), 0o755)); err != nil {
		os.Remove(f.Name())
		return "", err
	}
	return f.Name(), nil
}

// terminalScript is the shell command that runs the program from its folder
// with the engine's extra environment, then waits for Enter so the output
// stays readable after it exits.
func terminalScript(cmd *exec.Cmd) string {
	return "cd " + shQuote(cmd.Dir) + " && " + envPrefix(cmd.Env) + shJoin(cmd.Args) +
		`; code=$?; printf '\n\033[90m[exit %d] press Enter to close\033[0m' "$code"; read _`
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
