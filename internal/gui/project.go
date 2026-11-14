package gui

import (
	"bytes"
	"errors"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// A project's lessons share one folder, work/<project>/, that starts empty:
// you make every file in it. These are the project-only parts of the
// service: the order its lessons go in, and making, renaming and deleting
// files.

// ProjectState is where a project lesson stands in its project.
type ProjectState string

const (
	ProjectCurrent ProjectState = "current" // the lesson the project is on
	ProjectPassed  ProjectState = "passed"  // passed or caught up; readable, not run again
	ProjectLocked  ProjectState = "locked"  // the project hasn't reached it
)

// ProjectInfo is a project lesson's place in its project.
type ProjectInfo struct {
	ID     string       `json:"id"`
	Header string       `json:"header"` // the contract header, e.g. kite.hpp
	State  ProjectState `json:"state"`
	// OnID and OnTitle name the lesson the project is on; "" once every
	// lesson is passed.
	OnID    string `json:"onId"`
	OnTitle string `json:"onTitle"`
	// Contract is the declarations the tests use at this lesson, written
	// as you'd declare them in Header.
	Contract []string `json:"contract"`
	// Removes names the files this lesson's reference deletes.
	Removes []string `json:"removes"`
	// Starter names the files the project's first lesson puts in the
	// folder when it's made (data to try the program on); empty for the
	// other lessons, which never write into it.
	Starter []string `json:"starter"`
}

// projectInfo describes l's place in its project, or nil outside one. Hold mu.
func (s *Service) projectInfo(l *course.Lesson) *ProjectInfo {
	p := l.Project()
	if p == nil {
		return nil
	}
	info := &ProjectInfo{ID: p.ID, Header: p.Header, Removes: l.Meta.SolutionRemoves}
	for _, it := range l.Contract() {
		info.Contract = append(info.Contract, it.Decl())
	}
	if p.Lessons[0] == l {
		if starter, err := l.Starter(); err == nil {
			info.Starter = l.SortedNames(starter)
		}
	}
	cur := s.app.ProjectCurrent(p)
	if cur != nil {
		info.OnID, info.OnTitle = cur.ID, cur.Title
	}
	switch st := s.app.Progress.StatusOf(l.ID); {
	case cur == l:
		info.State = ProjectCurrent
	case st == store.Done || st == store.Skipped:
		info.State = ProjectPassed
	default:
		info.State = ProjectLocked
	}
	return info
}

// projectOrder sorts a project's file names: the contract header first,
// then by name.
func projectOrder(names []string, header string) []string {
	out := slices.Clone(names)
	slices.SortFunc(out, func(a, b string) int {
		switch {
		case a == header && b != header:
			return -1
		case b == header && a != header:
			return 1
		}
		return strings.Compare(a, b)
	})
	return out
}

// isText reports whether a file looks like text, as git decides: no NUL
// byte near the start. A program you built in the folder isn't shown.
func isText(data []byte) bool {
	return !bytes.Contains(data[:min(len(data), 8000)], []byte{0})
}

// projectFiles reads every text file in a project lesson's folder, read-only
// unless the project is on this lesson. A missing folder has none. Hold mu.
func (s *Service) projectFiles(l *course.Lesson) ([]File, error) {
	names, err := s.app.Engine.WorkFiles(l)
	if err != nil {
		return nil, err
	}
	work := s.app.Engine.WorkDir(l)
	readOnly := s.app.CheckOrder(l) != nil
	var out []File
	for _, name := range projectOrder(names, l.Project().Header) {
		data, err := os.ReadFile(filepath.Join(work, filepath.FromSlash(name)))
		if err != nil {
			if errors.Is(err, fs.ErrNotExist) {
				continue // deleted meanwhile
			}
			return nil, err
		}
		if !isText(data) {
			continue
		}
		out = append(out, File{Name: name, Content: string(data), Hash: hashOf(data), OnDisk: true, ReadOnly: readOnly})
	}
	return out, nil
}

// checkFileName checks a name you gave a file in a project: a slash path
// relative to the folder, without .., dot files or folders, or anything
// the engine treats as build output (and so leaves out of builds and sync).
func checkFileName(name string) error {
	switch {
	case strings.TrimSpace(name) == "":
		return errors.New("type a name for the file")
	case strings.ContainsAny(name, "\\\x00"):
		return fmt.Errorf("%q: use / between folders", name)
	case strings.HasPrefix(name, "/"):
		return fmt.Errorf("%q: name it from the project folder, like src/main.cpp", name)
	}
	parts := strings.Split(name, "/")
	for i, part := range parts {
		last := i == len(parts)-1
		switch {
		case part == "..":
			return fmt.Errorf("%q: a file can't go above the project folder", name)
		case part == "" || part == "." || strings.TrimSpace(part) == "":
			if last {
				return fmt.Errorf("%q needs a file name after the last /", name)
			}
			return fmt.Errorf("%q has an empty folder name", name)
		case strings.HasPrefix(part, "."):
			return fmt.Errorf("%q: names starting with a dot are left out of the project", name)
		case course.LeftOut(part, !last):
			return fmt.Errorf("%q: that's a name for build output, which the project leaves out", name)
		}
	}
	return nil
}

// projectPath is where a file you name goes in a project folder, checked
// with checkFileName and against links that lead out of the folder.
func projectPath(work, name string) (string, error) {
	if err := checkFileName(name); err != nil {
		return "", err
	}
	p := filepath.Join(work, filepath.FromSlash(name))
	root, err := filepath.EvalSymlinks(work)
	if err != nil {
		return "", err
	}
	// The nearest part of the path that exists decides where it really goes.
	at := p
	for {
		if _, err := os.Lstat(at); err == nil {
			break
		}
		at = filepath.Dir(at)
	}
	real, err := filepath.EvalSymlinks(at)
	if err != nil {
		return "", err
	}
	if real != root && !strings.HasPrefix(real, root+string(filepath.Separator)) {
		return "", fmt.Errorf("%q would land outside the project folder", name)
	}
	return p, nil
}

// projectLesson returns the lesson if it's a project lesson the project is
// on: the only one whose files you change. Hold mu.
func (s *Service) projectLesson(id string) (*course.Lesson, error) {
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	if l.Project() == nil {
		return nil, errors.New("only a project lesson has files you make yourself")
	}
	if err := s.app.CheckOrder(l); err != nil {
		return nil, err
	}
	return l, nil
}

// existingFile checks that name is a file you can rename or delete in the
// folder.
func existingFile(work, name string) (string, os.FileInfo, error) {
	if err := checkFileName(name); err != nil {
		return "", nil, err
	}
	st, err := os.Lstat(filepath.Join(work, filepath.FromSlash(name)))
	switch {
	case errors.Is(err, fs.ErrNotExist):
		return "", nil, fmt.Errorf("%s isn't in the project folder", name)
	case err != nil:
		return "", nil, err
	case !st.Mode().IsRegular():
		return "", nil, fmt.Errorf("%s isn't a file", name)
	}
	p, err := projectPath(work, name)
	return p, st, err
}

// CreateFile makes an empty file in a project lesson's folder: name is a
// slash path relative to it, and folders on the way are made. It never
// overwrites a file. The first one makes the folder, pulling first as
// SaveFile does, and starts the lesson.
func (s *Service) CreateFile(id, name string) (*File, error) {
	s.mu.Lock()
	l, err := s.projectLesson(id)
	dir, engine := s.app.Dir, s.app.Engine
	s.mu.Unlock()
	if err != nil {
		return nil, err
	}
	if err := checkFileName(name); err != nil { // before making the folder
		return nil, err
	}
	work, err := app.PrepareWorkspace(dir, engine, l) // may pull: no lock
	if err != nil {
		return nil, err
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	p, err := projectPath(work, name)
	if err != nil {
		return nil, err
	}
	if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
		return nil, err
	}
	f, err := os.OpenFile(p, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0o644)
	if errors.Is(err, fs.ErrExist) {
		return nil, fmt.Errorf("%s already exists", name)
	}
	if err != nil {
		return nil, err
	}
	if err := f.Close(); err != nil {
		return nil, err
	}
	s.files.wrote(p, hashOf(nil))
	if err := s.app.Start(l); err != nil {
		return nil, err
	}
	s.changed()
	return &File{Name: name, Hash: hashOf(nil), OnDisk: true}, nil
}

// RenameFile renames (or moves) a file in a project lesson's folder. It
// never overwrites a file. It returns the file under its new name.
func (s *Service) RenameFile(id, from, to string) (*File, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.projectLesson(id)
	if err != nil {
		return nil, err
	}
	work := s.app.Engine.WorkDir(l)
	src, srcInfo, err := existingFile(work, from)
	if err != nil {
		return nil, err
	}
	dst, err := projectPath(work, to)
	if err != nil {
		return nil, err
	}
	if from == to {
		return nil, fmt.Errorf("%s already has that name", from)
	}
	// Only a new spelling of the same file (main.cpp → Main.cpp on a
	// case-insensitive disk) may find its new name taken.
	if st, err := os.Lstat(dst); err == nil && !os.SameFile(srcInfo, st) {
		return nil, fmt.Errorf("%s already exists", to)
	}
	data, err := os.ReadFile(src)
	if err != nil {
		return nil, err
	}
	if err := os.MkdirAll(filepath.Dir(dst), 0o755); err != nil {
		return nil, err
	}
	if err := os.Rename(src, dst); err != nil {
		return nil, err
	}
	s.files.moved(src, dst, hashOf(data))
	s.changed()
	return &File{Name: to, Content: string(data), Hash: hashOf(data), OnDisk: true}, nil
}

// DeleteFile moves a file out of a project lesson's folder into a new
// folder in .local/reset-backup/, so nothing is lost, and returns where it
// went.
func (s *Service) DeleteFile(id, name string) (string, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.projectLesson(id)
	if err != nil {
		return "", err
	}
	src, _, err := existingFile(s.app.Engine.WorkDir(l), name)
	if err != nil {
		return "", err
	}
	dst, err := s.app.Engine.MoveFileAside(l, name)
	if err != nil {
		return "", err
	}
	s.files.moved(src, "", "")
	s.changed()
	return dst, nil
}

// CaughtUp is what catching up did.
type CaughtUp struct {
	Backup string        `json:"backup"` // where your files went; "" if there were none
	Files  []File        `json:"files"`  // the folder now: the reference
	Lesson *LessonDetail `json:"lesson"` // the lesson, now skipped
}

// CatchUp replaces the project folder with the course's reference as of
// the end of this lesson, the one the project is on, and skips the lesson
// (no XP). Your files move to .local/reset-backup/ first.
func (s *Service) CatchUp(id string) (*CaughtUp, error) {
	s.mu.Lock()
	l, err := s.lesson(id)
	var backup string
	if err == nil {
		backup, err = s.app.CatchUp(l)
	}
	if err == nil {
		s.changed()
	}
	s.mu.Unlock()
	if err != nil {
		return nil, err
	}
	files, err := s.OpenFiles(id)
	if err != nil {
		return nil, err
	}
	d, err := s.Lesson(id)
	if err != nil {
		return nil, err
	}
	go s.emit(EvState, nil)
	return &CaughtUp{Backup: backup, Files: files, Lesson: d}, nil
}
