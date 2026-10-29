// sysroad is a terminal tracker for a systems-programming learning roadmap.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"

	tea "github.com/charmbracelet/bubbletea"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/gitsync"
	"github.com/KayraBulbul/sysroad/internal/store"
	"github.com/KayraBulbul/sysroad/internal/tui"
)

const usage = `sysroad — systems engineering roadmap tracker

Usage:
  sysroad                 full-screen browser/editor
  sysroad side            compact view for a terminal split
  sysroad status          one-line summary (tmux status bar, prompts)
  sysroad log <dur> [id]  log time, e.g. 45m or 1h30m (default: current step)
  sysroad sync            commit, pull and push progress now
  sysroad init [-remote URL | -clone URL]
                          create the data repo (first machine), or clone it (others)
  sysroad where           print the data directory

Data lives in $SYSROAD_DIR (default ~/.local/share/sysroad).
`

func main() {
	args := os.Args[1:]
	cmd := ""
	if len(args) > 0 {
		cmd, args = args[0], args[1:]
	}
	dir := app.DefaultDir()
	var err error
	switch cmd {
	case "", "full":
		err = runTUI(dir, false)
	case "side", "sidebar":
		err = runTUI(dir, true)
	case "status":
		err = runStatus(dir, args)
	case "log":
		err = runLog(dir, args)
	case "sync":
		err = runSync(dir)
	case "init":
		err = runInit(dir, args)
	case "where":
		fmt.Println(dir)
	case "help", "-h", "--help":
		fmt.Print(usage)
	default:
		fmt.Fprintf(os.Stderr, "unknown command %q\n\n%s", cmd, usage)
		os.Exit(2)
	}
	if err != nil {
		fmt.Fprintln(os.Stderr, "sysroad:", err)
		os.Exit(1)
	}
}

type syncer interface{ Dirty() bool }

func runTUI(dir string, side bool) error {
	a, err := app.Open(dir)
	if err != nil {
		return err
	}
	var m tea.Model
	if side {
		m = tui.NewSide(a)
	} else {
		m = tui.NewFull(a)
	}
	final, err := tea.NewProgram(m, tea.WithAltScreen()).Run()
	if err != nil {
		return err
	}
	if s, ok := final.(syncer); ok && s.Dirty() && gitsync.IsRepo(dir) {
		fmt.Fprint(os.Stderr, "sysroad: syncing… ")
		if err := syncWithRetry(dir, 20*time.Second); err != nil {
			fmt.Fprintln(os.Stderr, "failed:", err, "(changes are committed locally; run `sysroad sync` later)")
			return nil
		}
		fmt.Fprintln(os.Stderr, "done")
	}
	return nil
}

// syncWithRetry waits out a sync still running in a background goroutine.
func syncWithRetry(dir string, timeout time.Duration) error {
	ctx, cancel := context.WithTimeout(context.Background(), timeout)
	defer cancel()
	for {
		err := gitsync.Sync(ctx, dir)
		if !errors.Is(err, gitsync.ErrBusy) {
			return err
		}
		select {
		case <-ctx.Done():
			return err
		case <-time.After(250 * time.Millisecond):
		}
	}
}

func runSync(dir string) error {
	if _, err := app.Open(dir); err != nil {
		return err
	}
	if !gitsync.IsRepo(dir) {
		return errors.New(dir + " is not a git repo — run `sysroad init`")
	}
	if err := syncWithRetry(dir, time.Minute); err != nil {
		return err
	}
	if !gitsync.HasRemote(context.Background(), dir) {
		fmt.Println("committed locally (no remote configured)")
	} else {
		fmt.Println("synced")
	}
	return nil
}

func runStatus(dir string, args []string) error {
	fs := flag.NewFlagSet("status", flag.ExitOnError)
	width := fs.Int("width", 40, "max width of the step title")
	fs.Parse(args)

	a, err := app.Open(dir)
	if err != nil {
		return err
	}
	now := time.Now()
	streak, _ := a.Progress.Streak(now)
	parts := []string{fmt.Sprintf("streak %dd", streak), store.FormatMinutes(a.TodayMinutes(now))}
	if t := a.Timer; t != nil {
		parts = append(parts, "◷ "+short(a.Roadmap.TitleOf(t.ID), *width)+" "+store.FormatMinutes(int(t.Elapsed(now)/time.Minute)))
	} else if cur, ok := a.Current(); ok {
		parts = append(parts, "> "+short(cur.Step.Title, *width))
	}
	fmt.Println(strings.Join(parts, " · "))
	return nil
}

func short(s string, n int) string {
	r := []rune(s)
	if n > 1 && len(r) > n {
		return string(r[:n-1]) + "…"
	}
	return s
}

func runLog(dir string, args []string) error {
	if len(args) < 1 {
		return errors.New("usage: sysroad log <duration> [step-id]")
	}
	mins, err := store.ParseMinutes(args[0])
	if err != nil {
		return err
	}
	a, err := app.Open(dir)
	if err != nil {
		return err
	}
	id := ""
	if len(args) > 1 {
		id = args[1]
		if a.Roadmap.Section(id) == nil {
			if _, st := a.Roadmap.Find(id); st == nil {
				return fmt.Errorf("no section or step with id %q", id)
			}
		}
	} else if cur, ok := a.Current(); ok {
		id = cur.Step.ID
	} else {
		return errors.New("no current step; pass a step id")
	}
	if err := a.LogTime(id, mins); err != nil {
		return err
	}
	fmt.Printf("logged %s on %s\n", store.FormatMinutes(mins), a.Roadmap.TitleOf(id))
	return nil
}

func runInit(dir string, args []string) error {
	fs := flag.NewFlagSet("init", flag.ExitOnError)
	remote := fs.String("remote", "", "git remote to push the new data repo to (first machine)")
	clone := fs.String("clone", "", "clone an existing data repo (additional machines)")
	fs.Parse(args)
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Minute)
	defer cancel()

	if _, err := os.Stat(dir); err == nil {
		if entries, _ := os.ReadDir(dir); len(entries) > 0 {
			return fmt.Errorf("%s already exists and isn't empty", dir)
		}
	}
	if *clone != "" {
		if err := gitsync.Clone(ctx, *clone, dir); err != nil {
			return err
		}
		if _, err := app.Open(dir); err != nil {
			return fmt.Errorf("cloned, but it doesn't look like sysroad data: %w", err)
		}
		fmt.Println("cloned progress into", dir)
		return nil
	}
	if err := app.Seed(dir); err != nil {
		return err
	}
	if err := gitsync.Init(ctx, dir, *remote); err != nil {
		return err
	}
	fmt.Println("created", dir)
	if *remote == "" {
		fmt.Println(`
No remote yet. To sync between machines, create a private repo and push, e.g.:
  gh repo create sysroad-data --private --source "` + dir + `" --push
Then on the other machine:
  sysroad init -clone <repo-url>`)
	}
	return nil
}
