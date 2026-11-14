// Package app ties the course, the learner's own roadmap sections, the
// progress log and the local timer together for a single data directory.
package app

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/roadmap"
	"github.com/KayraBulbul/sysroad/internal/store"
)

const (
	CustomFile = "custom.yaml" // the learner's own sections
	EventsFile = "events.jsonl"
	OldRoadmap = "roadmap.yaml" // pre-course data layout, migrated on open
	ArchiveDir = "archive"
	LocalDir   = ".local" // machine-local, gitignored
	TimerFile  = "timer.json"
)

// DefaultDir is the data repo location, overridable with $SYSROAD_DIR.
func DefaultDir() string {
	if d := os.Getenv("SYSROAD_DIR"); d != "" {
		return d
	}
	home, _ := os.UserHomeDir()
	return filepath.Join(home, ".local", "share", "sysroad")
}

type App struct {
	Dir      string
	Course   *course.Course
	Engine   *course.Engine
	Roadmap  *roadmap.Roadmap // course chapters followed by the learner's own sections
	Progress *store.Progress
	Timer    *store.Timer

	courseSecs                     []*roadmap.Section
	customTitle                    string
	customMod, eventsMod, timerMod time.Time
}

func (a *App) path(name string) string { return filepath.Join(a.Dir, name) }
func (a *App) timerPath() string       { return filepath.Join(a.Dir, LocalDir, TimerFile) }

var ErrNotInitialised = errors.New("no sysroad data found — run `sysroad init` first")

func exists(p string) bool { _, err := os.Stat(p); return err == nil }

func Open(dir string) (*App, error) {
	if !exists(filepath.Join(dir, EventsFile)) && !exists(filepath.Join(dir, CustomFile)) && !exists(filepath.Join(dir, OldRoadmap)) {
		return nil, ErrNotInitialised
	}
	c, err := course.Load()
	if err != nil {
		return nil, err
	}
	a := &App{Dir: dir, Course: c, Engine: course.NewEngine(dir), courseSecs: c.Roadmap().Sections}
	if err := a.migrate(); err != nil {
		return nil, fmt.Errorf("migrating to the course layout: %w", err)
	}
	if _, err := a.Reload(true); err != nil {
		return nil, err
	}
	return a, nil
}

const customSeed = "title: My own steps\nsections: []\n"

// gitignore keeps machine-local state and build output out of the data repo.
var gitignore = []string{LocalDir + "/", "*.tmp", "*.o", "a.out", "build/", ".DS_Store"}

// Seed writes the repo plumbing into an empty dir.
func Seed(dir string) error {
	if err := os.MkdirAll(dir, 0o755); err != nil {
		return err
	}
	files := map[string][]byte{
		CustomFile:       []byte(customSeed),
		EventsFile:       nil,
		".gitattributes": []byte(EventsFile + " merge=union\n"),
		".gitignore":     []byte(strings.Join(gitignore, "\n") + "\n"),
	}
	for name, data := range files {
		p := filepath.Join(dir, name)
		if _, err := os.Stat(p); err == nil {
			continue
		}
		if err := os.WriteFile(p, data, 0o644); err != nil {
			return err
		}
	}
	return nil
}

func mtime(p string) time.Time {
	st, err := os.Stat(p)
	if err != nil {
		return time.Time{}
	}
	return st.ModTime()
}

// Reload re-reads files whose mtime changed (another process, or a git pull).
func (a *App) Reload(force bool) (bool, error) {
	changed := false
	if m := mtime(a.path(CustomFile)); force || !m.Equal(a.customMod) {
		custom := &roadmap.Roadmap{}
		if exists(a.path(CustomFile)) {
			r, err := roadmap.Load(a.path(CustomFile))
			if err != nil {
				return false, err
			}
			custom = r
		}
		merged := &roadmap.Roadmap{Title: a.Course.Title}
		merged.Sections = append(append(merged.Sections, a.courseSecs...), custom.Sections...)
		if err := merged.Validate(); err != nil {
			return false, fmt.Errorf("%s: %w (an ID clashes with the course)", CustomFile, err)
		}
		a.Roadmap, a.customTitle, a.customMod, changed = merged, custom.Title, m, true
	}
	if m := mtime(a.path(EventsFile)); force || !m.Equal(a.eventsMod) {
		ev, err := store.ReadEvents(a.path(EventsFile))
		if err != nil {
			return false, err
		}
		a.Progress, a.eventsMod, changed = store.Replay(ev), m, true
	}
	if m := mtime(a.timerPath()); force || !m.Equal(a.timerMod) {
		a.Timer, a.timerMod, changed = store.ReadTimer(a.timerPath()), m, true
	}
	return changed, nil
}

func (a *App) append(e store.Event) error {
	if err := store.Append(a.path(EventsFile), e); err != nil {
		return err
	}
	_, err := a.Reload(false)
	return err
}

func (a *App) SetStatus(id string, s store.Status) error {
	if a.Progress.StatusOf(id) == s {
		return nil
	}
	return a.append(store.Event{Type: store.EvStatus, ID: id, Value: string(s)})
}

func (a *App) SetNote(id, text string) error {
	if a.Progress.Notes[id] == text {
		return nil
	}
	return a.append(store.Event{Type: store.EvNote, ID: id, Text: text})
}

func (a *App) LogTime(id string, minutes int) error {
	if minutes <= 0 {
		return nil
	}
	return a.append(store.Event{Type: store.EvTime, ID: id, Minutes: minutes})
}

// SaveRoadmap writes the learner's own (non-course) sections to custom.yaml.
func (a *App) SaveRoadmap() error {
	custom := &roadmap.Roadmap{Title: a.customTitle}
	if custom.Title == "" {
		custom.Title = "My own steps"
	}
	for _, s := range a.Roadmap.Sections {
		if !s.ReadOnly {
			custom.Sections = append(custom.Sections, s)
		}
	}
	if custom.Sections == nil {
		custom.Sections = []*roadmap.Section{}
	}
	if err := custom.Save(a.path(CustomFile)); err != nil {
		return err
	}
	a.customMod = mtime(a.path(CustomFile))
	return nil
}

// Lesson returns the course lesson for a step ID, or nil.
func (a *App) Lesson(id string) *course.Lesson { return a.Course.Lesson(id) }

// XP is the total earned from completed course lessons.
func (a *App) XP() int {
	xp := 0
	for _, l := range a.Course.Lessons() {
		if a.Progress.StatusOf(l.ID) == store.Done {
			xp += course.XPFor(l)
		}
	}
	return xp
}

// StartTimer starts timing id, stopping (and logging) any running timer first.
// A todo step becomes "doing".
func (a *App) StartTimer(id string) error {
	if _, err := a.StopTimer(); err != nil {
		return err
	}
	if err := os.MkdirAll(filepath.Join(a.Dir, LocalDir), 0o755); err != nil {
		return err
	}
	if err := store.WriteTimer(a.timerPath(), &store.Timer{ID: id, Start: time.Now()}); err != nil {
		return err
	}
	if _, st := a.Roadmap.Find(id); st != nil && a.Progress.StatusOf(id) == store.Todo {
		if err := a.SetStatus(id, store.Doing); err != nil {
			return err
		}
	}
	_, err := a.Reload(false)
	return err
}

// StopTimer logs the running timer's elapsed minutes. Returns minutes logged.
func (a *App) StopTimer() (int, error) {
	t := store.ReadTimer(a.timerPath())
	if t == nil {
		return 0, nil
	}
	mins := int(t.Elapsed(time.Now()).Round(time.Minute) / time.Minute)
	if err := store.WriteTimer(a.timerPath(), nil); err != nil {
		return 0, err
	}
	if mins < 1 {
		mins = 1
	}
	if err := a.LogTime(t.ID, mins); err != nil {
		return 0, err
	}
	_, err := a.Reload(false)
	return mins, err
}

// Ref locates a step in the roadmap.
type Ref struct {
	Sec  *roadmap.Section
	Step *roadmap.Step
}

// Counts reports whether a step counts towards its section's progress:
// optional steps only count once done, skipped steps never count.
func (a *App) Counts(st *roadmap.Step) bool {
	s := a.Progress.StatusOf(st.ID)
	if s == store.Skipped {
		return false
	}
	return !st.Optional || s == store.Done
}

// Available is false for course lessons that aren't written yet, so the
// current step never lands on one.
func (a *App) Available(st *roadmap.Step) bool {
	l := a.Lesson(st.ID)
	return l == nil || l.Written || l.Detail != "" || l.URL != ""
}

func (a *App) mainSteps() []Ref {
	var refs []Ref
	for _, sec := range a.Roadmap.Sections {
		if sec.Optional {
			continue
		}
		for _, st := range sec.Steps {
			if !st.Optional && a.Available(st) {
				refs = append(refs, Ref{sec, st})
			}
		}
	}
	return refs
}

// Current is the most recently started "doing" step anywhere, else the first
// unfinished step on the main path.
func (a *App) Current() (Ref, bool) {
	var best Ref
	var bestAt time.Time
	for _, sec := range a.Roadmap.Sections {
		for _, st := range sec.Steps {
			if a.Progress.StatusOf(st.ID) == store.Doing {
				if at := a.Progress.StatusAt[st.ID]; best.Step == nil || at.After(bestAt) {
					best, bestAt = Ref{sec, st}, at
				}
			}
		}
	}
	if best.Step != nil {
		return best, true
	}
	for _, r := range a.mainSteps() {
		if a.Progress.StatusOf(r.Step.ID) == store.Todo {
			return r, true
		}
	}
	return Ref{}, false
}

// Doing returns all in-progress steps other than exclude.
func (a *App) Doing(exclude string) []Ref {
	var out []Ref
	for _, sec := range a.Roadmap.Sections {
		for _, st := range sec.Steps {
			if st.ID != exclude && a.Progress.StatusOf(st.ID) == store.Doing {
				out = append(out, Ref{sec, st})
			}
		}
	}
	return out
}

// Upcoming lists the next n todo steps on the main path after the current one.
func (a *App) Upcoming(n int) []Ref {
	cur, _ := a.Current()
	var out []Ref
	for _, r := range a.mainSteps() {
		if len(out) >= n {
			break
		}
		if (cur.Step != nil && r.Step.ID == cur.Step.ID) || a.Progress.StatusOf(r.Step.ID) != store.Todo {
			continue
		}
		out = append(out, r)
	}
	return out
}

// RecentDone lists the n most recently completed steps, newest first.
func (a *App) RecentDone(n int) []Ref {
	var out []Ref
	seen := map[string]bool{}
	ev := a.Progress.Events
	for i := len(ev) - 1; i >= 0 && len(out) < n; i-- {
		e := ev[i]
		if e.Type != store.EvStatus || seen[e.ID] {
			continue
		}
		seen[e.ID] = true
		if a.Progress.StatusOf(e.ID) != store.Done {
			continue
		}
		if sec, st := a.Roadmap.Find(e.ID); st != nil {
			out = append(out, Ref{sec, st})
		}
	}
	return out
}

type Stats struct {
	Done, Total    int
	Minutes        int     // logged on the section and its steps
	HoursLeft      float64 // estimated, from step hours
	HoursEstimated float64
}

func (s Stats) Frac() float64 {
	if s.Total == 0 {
		return 0
	}
	return float64(s.Done) / float64(s.Total)
}

func (a *App) SectionStats(sec *roadmap.Section) Stats {
	st := Stats{Minutes: a.Progress.Minutes[sec.ID]}
	for _, step := range sec.Steps {
		st.Minutes += a.Progress.Minutes[step.ID]
		if !a.Counts(step) {
			continue
		}
		st.Total++
		st.HoursEstimated += step.Hours
		if a.Progress.StatusOf(step.ID) == store.Done {
			st.Done++
		} else {
			st.HoursLeft += step.Hours
		}
	}
	return st
}

// Overall aggregates non-optional sections.
func (a *App) Overall() Stats {
	var t Stats
	for _, sec := range a.Roadmap.Sections {
		s := a.SectionStats(sec)
		t.Minutes += s.Minutes
		if sec.Optional {
			continue
		}
		t.Done += s.Done
		t.Total += s.Total
		t.HoursLeft += s.HoursLeft
		t.HoursEstimated += s.HoursEstimated
	}
	return t
}

// ETA estimates time left on the main path at the 14-day average pace.
func (a *App) ETA(now time.Time) string {
	avg := a.Progress.AvgMinutes(now, 14)
	left := a.Overall().HoursLeft
	if avg < 1 || left == 0 {
		return ""
	}
	days := left * 60 / avg
	switch {
	case days < 14:
		return fmt.Sprintf("~%.0f days", days)
	case days < 120:
		return fmt.Sprintf("~%.0f weeks", days/7)
	case days < 540:
		return fmt.Sprintf("~%.0f months", days/30.4)
	default:
		return fmt.Sprintf("~%.1f years", days/365)
	}
}

func (a *App) TodayMinutes(now time.Time) int {
	m := a.Progress.DayMins[store.Day(now)]
	if a.Timer != nil {
		m += int(a.Timer.Elapsed(now) / time.Minute)
	}
	return m
}
