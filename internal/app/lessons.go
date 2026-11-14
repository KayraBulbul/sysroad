package app

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/gitsync"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// ResolveLesson picks the lesson a command means: an explicit ID, else the
// lesson whose work folder contains cwd (in a project's folder, the lesson
// the project is on), else the current step if it is a lesson, else the
// first unfinished written lesson.
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
				if p := a.Course.Project(id); p != nil {
					return a.ProjectLesson(p), nil
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
		if l.Written && a.Progress.StatusOf(l.ID) != store.Done && a.CheckOrder(l) == nil {
			return l, nil
		}
	}
	return nil, errors.New("no lesson to work on — pass a lesson id")
}

// PrepareWorkspace creates a lesson's folder from its starter files. The
// first time this machine creates one, it pulls first, so a folder the other
// machine already pushed is used rather than a fresh starter copy, which
// would conflict on the next sync. It doesn't touch App state, so it's safe
// to call from a background goroutine.
func PrepareWorkspace(dir string, e *course.Engine, l *course.Lesson) (string, error) {
	work := e.WorkDir(l)
	if l.HasCode() && !exists(work) && gitsync.IsRepo(dir) {
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		remote := gitsync.HasRemote(ctx, dir)
		cancel()
		if remote {
			gitsync.SyncWait(dir, 15*time.Second) // best effort: offline is fine
		}
	}
	return e.Ensure(l)
}

// Why a lesson can't be run: front ends word these for their own keys.
var (
	ErrManualStep = errors.New("nothing to run here — mark it done when you've done it")
	ErrUnwritten  = errors.New("this lesson isn't written yet")
	ErrQuiz       = errors.New("this is a quiz — pick an answer for each question")
)

// Runnable reports whether Run/Submit applies to a lesson.
func Runnable(l *course.Lesson) error {
	switch {
	case l.IsQuiz():
		return ErrQuiz
	case l.Graded():
		return nil
	case l.Written || l.Detail != "" || l.URL != "":
		return ErrManualStep
	default:
		return ErrUnwritten
	}
}

// BeginRun checks a lesson can be run and marks it in progress. Follow it
// with Execute off the main thread, then Record.
func (a *App) BeginRun(l *course.Lesson) error {
	if err := Runnable(l); err != nil {
		return err
	}
	if err := a.CheckOrder(l); err != nil {
		return err
	}
	return a.Start(l)
}

// Execute prepares the lesson's workspace and runs it. Like
// PrepareWorkspace it doesn't touch App state, so it's safe to call from a
// background goroutine while the front end keeps using the App.
func Execute(ctx context.Context, dir string, e *course.Engine, l *course.Lesson, mode course.Mode) (*course.Result, error) {
	if _, err := PrepareWorkspace(dir, e, l); err != nil {
		return nil, err
	}
	return e.Run(ctx, l, mode)
}

// ToggleStatus sets a step's status, or puts it back to todo if it already
// has it. Finishing a step stops its timer.
func (a *App) ToggleStatus(id string, s store.Status) error {
	cur := a.Progress.StatusOf(id)
	if l := a.Lesson(id); l != nil && l.Project() != nil {
		if s == store.Done || s == store.Skipped || cur == store.Done || cur == store.Skipped {
			return ErrProjectStatus
		}
		if err := a.CheckOrder(l); err != nil {
			return err
		}
	}
	if cur == s {
		s = store.Todo
	}
	if s == store.Done {
		if t := a.Timer; t != nil && t.ID == id {
			if _, err := a.StopTimer(); err != nil {
				return err
			}
		}
	}
	return a.SetStatus(id, s)
}

// Start marks a todo lesson in progress. A project lesson starts only when
// the project gets to it.
func (a *App) Start(l *course.Lesson) error {
	if a.Progress.StatusOf(l.ID) == store.Todo && a.CheckOrder(l) == nil {
		return a.SetStatus(l.ID, store.Doing)
	}
	return nil
}

// Touch creates the lesson's workspace and marks a todo lesson in progress.
func (a *App) Touch(l *course.Lesson) (string, error) {
	dir, err := PrepareWorkspace(a.Dir, a.Engine, l)
	if err != nil {
		return "", err
	}
	if _, err := a.Reload(false); err != nil { // the pull may have brought new events
		return "", err
	}
	return dir, a.Start(l)
}

// Outcome is what changed when a Run/Submit result was recorded.
type Outcome struct {
	Completed bool `json:"completed"` // this submit completed the lesson
	XPGained  int  `json:"xpGained"`
	LevelUp   bool `json:"levelUp"`
	Level     int  `json:"level"`
	XP        int  `json:"xp"`
	Into      int  `json:"into"`
	Span      int  `json:"span"`
}

// RunLesson runs or submits a lesson and records a passing Submit.
func (a *App) RunLesson(ctx context.Context, l *course.Lesson, mode course.Mode) (*course.Result, Outcome, error) {
	if err := a.CheckOrder(l); err != nil {
		return nil, Outcome{}, err
	}
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

// Record marks the lesson done if the result is a passing Submit. Passing
// a project lesson also saves a snapshot of the project folder, which the
// next lesson's Reset goes back to.
func (a *App) Record(l *course.Lesson, r *course.Result) (Outcome, error) {
	before := a.XP()
	completed := false
	if r.OK && r.Mode == course.ModeSubmit && a.Progress.StatusOf(l.ID) != store.Done && a.CheckOrder(l) == nil {
		if l.Project() != nil {
			if err := a.Engine.SaveSnapshot(l); err != nil {
				return Outcome{}, err
			}
		}
		if err := a.SetStatus(l.ID, store.Done); err != nil {
			return Outcome{}, err
		}
		completed = true
	}
	return a.outcome(before, completed), nil
}

// outcome describes the XP change since `before`.
func (a *App) outcome(before int, completed bool) Outcome {
	beforeLvl, _, _ := course.Level(before)
	o := Outcome{Completed: completed, XP: a.XP()}
	o.XPGained = o.XP - before
	o.Level, o.Into, o.Span = course.Level(o.XP)
	o.LevelUp = o.Level > beforeLvl
	return o
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
