package app

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// ResolveLesson picks the lesson a command means: an explicit ID, else the
// lesson whose work folder contains cwd, else the current step if it is a
// lesson, else the first unfinished written lesson.
func (a *App) ResolveLesson(arg, cwd string) (*course.Lesson, error) {
	if arg != "" {
		if l := a.Lesson(arg); l != nil {
			return l, nil
		}
		return nil, fmt.Errorf("no lesson with id %q", arg)
	}
	if cwd != "" {
		work := filepath.Join(a.Dir, course.WorkDirName) + string(filepath.Separator)
		if abs, err := filepath.Abs(cwd); err == nil {
			if real, err := filepath.EvalSymlinks(abs); err == nil {
				abs = real
			}
			if realWork, err := filepath.EvalSymlinks(strings.TrimSuffix(work, string(filepath.Separator))); err == nil {
				work = realWork + string(filepath.Separator)
			}
			if rest, ok := strings.CutPrefix(abs+string(filepath.Separator), work); ok {
				id := strings.SplitN(rest, string(filepath.Separator), 2)[0]
				if l := a.Lesson(id); l != nil {
					return l, nil
				}
			}
		}
	}
	if cur, ok := a.Current(); ok {
		if l := a.Lesson(cur.Step.ID); l != nil {
			return l, nil
		}
	}
	for _, l := range a.Course.Lessons() {
		if l.Written && a.Progress.StatusOf(l.ID) != store.Done {
			return l, nil
		}
	}
	return nil, errors.New("no lesson to work on — pass a lesson id")
}

// Touch creates the lesson's workspace and marks a todo lesson in progress.
func (a *App) Touch(l *course.Lesson) (string, error) {
	dir, err := a.Engine.Ensure(l)
	if err != nil {
		return "", err
	}
	if a.Progress.StatusOf(l.ID) == store.Todo {
		if err := a.SetStatus(l.ID, store.Doing); err != nil {
			return "", err
		}
	}
	return dir, nil
}

// Outcome is what changed when a Run/Submit result was recorded.
type Outcome struct {
	Completed  bool // this submit completed the lesson
	XPGained   int
	LevelUp    bool
	Level, XP  int
	Into, Span int
}

// RunLesson runs or submits a lesson and records a passing Submit.
func (a *App) RunLesson(ctx context.Context, l *course.Lesson, mode course.Mode) (*course.Result, Outcome, error) {
	if _, err := a.Touch(l); err != nil {
		return nil, Outcome{}, err
	}
	r, err := a.Engine.Run(ctx, l, mode)
	if err != nil {
		return nil, Outcome{}, err
	}
	out, err := a.Record(l, r)
	return r, out, err
}

// Record marks the lesson done if the result is a passing Submit.
func (a *App) Record(l *course.Lesson, r *course.Result) (Outcome, error) {
	before := a.XP()
	beforeLvl, _, _ := course.Level(before)
	var o Outcome
	if r.OK && r.Mode == course.ModeSubmit && a.Progress.StatusOf(l.ID) != store.Done {
		if err := a.SetStatus(l.ID, store.Done); err != nil {
			return o, err
		}
		o.Completed = true
	}
	o.XP = a.XP()
	o.XPGained = o.XP - before
	o.Level, o.Into, o.Span = course.Level(o.XP)
	o.LevelUp = o.Level > beforeLvl
	return o, nil
}

// ---- hints (machine-local: .local/hints.json) ----

func (a *App) hintsPath() string { return filepath.Join(a.Dir, LocalDir, "hints.json") }

func (a *App) hintState() map[string]int {
	m := map[string]int{}
	if data, err := os.ReadFile(a.hintsPath()); err == nil {
		json.Unmarshal(data, &m)
	}
	return m
}

// HintsShown is how many of a lesson's hints have been revealed.
func (a *App) HintsShown(id string) int { return a.hintState()[id] }

// RevealHint reveals the next hint and returns how many are now shown.
func (a *App) RevealHint(l *course.Lesson) (int, error) {
	m := a.hintState()
	n := min(m[l.ID]+1, len(l.Hints()))
	m[l.ID] = n
	if err := os.MkdirAll(filepath.Dir(a.hintsPath()), 0o755); err != nil {
		return 0, err
	}
	data, _ := json.Marshal(m)
	return n, os.WriteFile(a.hintsPath(), data, 0o644)
}

// HintsModTime lets views notice hints revealed by another process.
func (a *App) HintsModTime() int64 {
	st, err := os.Stat(a.hintsPath())
	if err != nil {
		return 0
	}
	return st.ModTime().UnixNano()
}
