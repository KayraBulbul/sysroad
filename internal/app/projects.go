package app

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"time"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// A project's lessons unlock one at a time. Run, Submit, Reset and
// catching up act only on the project's current lesson: the first one not
// passed or skipped. Passed lessons stay readable but aren't run again.

// ProjectCurrent is the lesson a project is on, or nil once every lesson
// is passed or skipped.
func (a *App) ProjectCurrent(p *course.Project) *course.Lesson {
	for _, l := range p.Lessons {
		if s := a.Progress.StatusOf(l.ID); s != store.Done && s != store.Skipped {
			return l
		}
	}
	return nil
}

// ProjectLesson is the lesson to show for a project: its current lesson,
// or its last once every lesson is passed.
func (a *App) ProjectLesson(p *course.Project) *course.Lesson {
	if l := a.ProjectCurrent(p); l != nil {
		return l
	}
	return p.Lessons[len(p.Lessons)-1]
}

// OrderError says a project lesson can't be acted on: its project is on
// another lesson (Current), or finished (Current nil).
type OrderError struct {
	Lesson  *course.Lesson
	Current *course.Lesson
}

func (e *OrderError) Error() string {
	p := e.Lesson.Project()
	switch {
	case e.Current == nil:
		return fmt.Sprintf("you've finished the %s project", p.ID)
	case slices.Index(p.Lessons, e.Lesson) < slices.Index(p.Lessons, e.Current):
		return fmt.Sprintf("you've passed %q; the %s project is on %q now", e.Lesson.Title, p.ID, e.Current.Title)
	default:
		return fmt.Sprintf("%q is locked: the %s project is on %q", e.Lesson.Title, p.ID, e.Current.Title)
	}
}

// CheckOrder returns an *OrderError unless l is outside a project or is
// its project's current lesson.
func (a *App) CheckOrder(l *course.Lesson) error {
	p := l.Project()
	if p == nil {
		return nil
	}
	if cur := a.ProjectCurrent(p); cur != l {
		return &OrderError{Lesson: l, Current: cur}
	}
	return nil
}

// checkLocked returns an *OrderError if id is a project lesson the project
// hasn't reached yet.
func (a *App) checkLocked(id string) error {
	l := a.Lesson(id)
	if l == nil || l.Project() == nil {
		return nil
	}
	if s := a.Progress.StatusOf(id); s == store.Done || s == store.Skipped {
		return nil
	}
	return a.CheckOrder(l)
}

// ErrProjectStatus: ticking a project lesson off by hand would skip strict
// order, and un-ticking one would leave the folder ahead of the lesson.
var ErrProjectStatus = errors.New("a project lesson is passed by Submit, or skipped by catching up to the reference")

// ResetLesson moves the lesson's files to .local/reset-backup/ and puts the
// starter back, or for a project lesson the folder as it was at the start
// of the lesson. It returns where the old files went.
func (a *App) ResetLesson(l *course.Lesson) (string, error) {
	if !l.HasCode() {
		return "", errors.New("this lesson has no code to reset")
	}
	if err := a.CheckOrder(l); err != nil {
		return "", err
	}
	return a.Engine.Reset(l)
}

// CatchUp replaces the project folder with the reference as of the end of
// l, the project's current lesson, and marks l skipped so the project moves
// on. Your files move to .local/reset-backup/ first; the returned path is
// where they went. It's the only way past a lesson you haven't passed.
func (a *App) CatchUp(l *course.Lesson) (string, error) {
	if l.Project() == nil {
		return "", errors.New("only a project lesson can catch up to the reference")
	}
	if err := a.CheckOrder(l); err != nil {
		return "", err
	}
	backup, err := a.Engine.UseReference(l)
	if err != nil {
		return backup, err
	}
	// The next lesson's Reset starts from this snapshot.
	if err := a.Engine.SaveSnapshot(l); err != nil {
		return backup, err
	}
	return backup, a.SetStatus(l.ID, store.Skipped)
}

// EditFile is the file `sysroad edit` opens, relative to the lesson's
// folder: the lesson's first file, or in a project the source or header
// you changed last (the contract header while there are none).
func (a *App) EditFile(l *course.Lesson) (string, error) {
	p := l.Project()
	if p == nil {
		if len(l.Meta.Files) == 0 {
			return "", fmt.Errorf("%q has no files to edit", l.Title)
		}
		return l.Meta.Files[0], nil
	}
	files, err := a.Engine.WorkFiles(l)
	if err != nil {
		return "", err
	}
	work := a.Engine.WorkDir(l)
	best, bestAt := p.Header, time.Time{}
	for _, f := range files {
		switch filepath.Ext(f) {
		case ".cpp", ".cc", ".hpp", ".h":
		default:
			continue
		}
		if st, err := os.Stat(filepath.Join(work, filepath.FromSlash(f))); err == nil && st.ModTime().After(bestAt) {
			best, bestAt = f, st.ModTime()
		}
	}
	return best, nil
}
