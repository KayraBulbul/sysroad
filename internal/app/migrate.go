package app

import (
	"os"
	"path/filepath"
	"strings"

	"github.com/KayraBulbul/sysroad/internal/store"
)

// carryOver maps finished work from the old C roadmap onto the course.
var carryOver = []struct {
	oldID, newID string
	status       bool // carry a "done" status
	note         bool // carry the note
}{
	{"setup-2", "setup-toolchain", true, false},       // toolchain installed
	{"setup-1", "side-missing-semester", false, true}, // Missing Semester, part-watched
}

// migrate moves a pre-course data repo (roadmap.yaml) to the course layout:
// the old roadmap is archived, custom.yaml is created, and the few steps that
// still apply carry their progress over. Old events stay in the log, so
// streaks and logged time are kept. Safe to run on every open.
func (a *App) migrate() error {
	old := a.path(OldRoadmap)
	if exists(old) {
		if err := os.MkdirAll(a.path(ArchiveDir), 0o755); err != nil {
			return err
		}
		dst := filepath.Join(a.path(ArchiveDir), "roadmap-c.yaml")
		if exists(dst) {
			dst = filepath.Join(a.path(ArchiveDir), "roadmap-c-"+hostnameSafe()+".yaml")
		}
		if err := os.Rename(old, dst); err != nil {
			return err
		}
		events, err := store.ReadEvents(a.path(EventsFile))
		if err != nil {
			return err
		}
		p := store.Replay(events)
		for _, c := range carryOver {
			if c.status && p.StatusOf(c.oldID) == store.Done && p.StatusOf(c.newID) != store.Done {
				if err := store.Append(a.path(EventsFile), store.Event{Type: store.EvStatus, ID: c.newID, Value: string(store.Done)}); err != nil {
					return err
				}
			}
			if n := p.Notes[c.oldID]; c.note && n != "" && p.Notes[c.newID] == "" {
				if err := store.Append(a.path(EventsFile), store.Event{Type: store.EvNote, ID: c.newID, Text: n}); err != nil {
					return err
				}
			}
		}
	}
	if !exists(a.path(CustomFile)) && exists(a.path(EventsFile)) {
		if err := os.WriteFile(a.path(CustomFile), []byte(customSeed), 0o644); err != nil {
			return err
		}
	}
	return ensureGitignore(a.path(".gitignore"))
}

func ensureGitignore(p string) error {
	if !exists(filepath.Dir(p) + "/.git") {
		return nil
	}
	data, _ := os.ReadFile(p)
	have := map[string]bool{}
	for _, l := range strings.Split(string(data), "\n") {
		have[strings.TrimSpace(l)] = true
	}
	var add []string
	for _, l := range gitignore {
		if !have[l] {
			add = append(add, l)
		}
	}
	if len(add) == 0 {
		return nil
	}
	s := string(data)
	if s != "" && !strings.HasSuffix(s, "\n") {
		s += "\n"
	}
	return os.WriteFile(p, []byte(s+strings.Join(add, "\n")+"\n"), 0o644)
}

func hostnameSafe() string {
	h, _ := os.Hostname()
	if i := strings.IndexByte(h, '.'); i > 0 {
		h = h[:i]
	}
	if h == "" {
		h = "other"
	}
	return h
}
