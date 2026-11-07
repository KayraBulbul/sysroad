package gui

import (
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"sync"
	"time"

	"github.com/fsnotify/fsnotify"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
)

// File is one lesson file as the editor sees it. Hash identifies the
// content the editor started from, so a save can tell whether the file
// changed underneath it (the other machine syncing, or an edit in nvim).
type File struct {
	Name    string `json:"name"`
	Content string `json:"content"`
	Hash    string `json:"hash"`
	OnDisk  bool   `json:"onDisk"` // false: starter code, the folder isn't made yet
}

// FileEvent reports a lesson file that changed on disk.
type FileEvent struct {
	Lesson string `json:"lesson"`
	File
}

type ResultEvent struct {
	Lesson string `json:"lesson"`
}

func hashOf(b []byte) string {
	h := sha256.Sum256(b)
	return hex.EncodeToString(h[:8])
}

// editable returns the lesson if name is one of the files you edit in it.
func (s *Service) editable(id, name string) (*course.Lesson, error) {
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	if !l.HasCode() {
		return nil, errors.New("this lesson has no code to edit")
	}
	if !slices.Contains(l.Meta.Files, name) {
		return nil, fmt.Errorf("%q isn't one of this lesson's files", name)
	}
	return l, nil
}

// OpenFiles returns the files you edit in a lesson and starts watching them.
// A lesson you haven't started yet shows its starter code; the folder is made
// on the first save or run, so browsing doesn't litter the data repo.
func (s *Service) OpenFiles(id string) ([]File, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	if !l.HasCode() {
		s.files.watch(nil)
		return nil, nil
	}
	starter, err := l.Starter()
	if err != nil {
		return nil, err
	}
	work := s.app.Engine.WorkDir(l)
	var out []File
	for _, name := range l.Meta.Files {
		f := File{Name: name}
		if data, err := os.ReadFile(filepath.Join(work, name)); err == nil {
			f.Content, f.Hash, f.OnDisk = string(data), hashOf(data), true
		} else {
			f.Content, f.Hash = string(starter[name]), hashOf(starter[name])
		}
		out = append(out, f)
	}
	s.files.watch(l)
	return out, nil
}

// SaveResult is what a save did. On a conflict nothing was written and Disk
// holds what's there now.
type SaveResult struct {
	Hash     string `json:"hash"`
	Conflict bool   `json:"conflict"`
	Disk     *File  `json:"disk"`
}

// SaveFile writes a lesson file if it still has the content the editor
// started from (base, a File.Hash). The first save makes the lesson's folder,
// pulling first so a folder the other machine pushed wins over the starter.
func (s *Service) SaveFile(id, name, content, base string) (*SaveResult, error) {
	s.mu.Lock()
	l, err := s.editable(id, name)
	dir, engine := s.app.Dir, s.app.Engine
	s.mu.Unlock()
	if err != nil {
		return nil, err
	}
	work, err := app.PrepareWorkspace(dir, engine, l) // may pull: no lock
	if err != nil {
		return nil, err
	}
	path := filepath.Join(work, name)

	s.mu.Lock()
	defer s.mu.Unlock()
	if disk, err := os.ReadFile(path); err == nil && hashOf(disk) != base {
		if hashOf(disk) == hashOf([]byte(content)) { // already saved elsewhere
			return &SaveResult{Hash: hashOf(disk)}, nil
		}
		return &SaveResult{Conflict: true, Disk: &File{Name: name, Content: string(disk), Hash: hashOf(disk), OnDisk: true}}, nil
	}
	data := []byte(content)
	// Write in place (not rename) so nvim and the file watcher see one file.
	if err := os.WriteFile(path, data, 0o644); err != nil {
		return nil, err
	}
	s.files.wrote(path, hashOf(data))
	if err := s.app.Start(l); err != nil {
		return nil, err
	}
	s.changed()
	return &SaveResult{Hash: hashOf(data)}, nil
}

// Reset puts the starter code back, moving your files to
// .local/reset-backup/. It returns the backup folder and the new files.
func (s *Service) Reset(id string) (string, []File, error) {
	s.mu.Lock()
	l, err := s.lesson(id)
	if err == nil && !l.HasCode() {
		err = errors.New("this lesson has no code to reset")
	}
	var backup string
	if err == nil {
		backup, err = s.app.Engine.Reset(l)
	}
	if err == nil {
		s.changed()
	}
	s.mu.Unlock()
	if err != nil {
		return "", nil, err
	}
	files, err := s.OpenFiles(id)
	return backup, files, err
}

// Solution is the lesson's reference solution, for the files you edit.
func (s *Service) Solution(id string) ([]File, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	if !l.HasCode() {
		return nil, errors.New("this lesson has no solution to show")
	}
	sol, err := l.Solution()
	if err != nil {
		return nil, err
	}
	var out []File
	for _, name := range l.SortedNames(sol) {
		out = append(out, File{Name: name, Content: string(sol[name]), Hash: hashOf(sol[name])})
	}
	return out, nil
}

// ---- watching ----

// fileWatcher follows the open lesson's folder. It watches the chain from
// the data folder down (data, work/, work/<lesson>) and re-adds it whenever
// one of those changes, so a folder that appears (first save, a git pull) or
// is replaced (Reset) keeps being followed.
type fileWatcher struct {
	engine *course.Engine
	notify func(FileEvent)

	mu      sync.Mutex
	w       *fsnotify.Watcher
	lesson  *course.Lesson
	known   map[string]string // path → last hash reported or written
	pending map[string]*time.Timer
}

func newFileWatcher(e *course.Engine, notify func(FileEvent)) *fileWatcher {
	fw := &fileWatcher{engine: e, notify: notify, known: map[string]string{}, pending: map[string]*time.Timer{}}
	w, err := fsnotify.NewWatcher()
	if err != nil {
		return fw // no live reload; saves still detect conflicts
	}
	fw.w = w
	go fw.loop()
	return fw
}

func (fw *fileWatcher) dirs() []string {
	work := fw.engine.WorkDir(fw.lesson)
	return []string{fw.engine.DataDir, filepath.Dir(work), work}
}

// rewatch (re)adds the chain; a folder that doesn't exist yet is skipped
// until its parent reports it. Hold fw.mu.
func (fw *fileWatcher) rewatch() {
	for _, d := range fw.dirs() {
		fw.w.Add(d)
	}
}

// watch switches to a lesson (nil: none).
func (fw *fileWatcher) watch(l *course.Lesson) {
	if fw.w == nil {
		return
	}
	fw.mu.Lock()
	defer fw.mu.Unlock()
	if fw.lesson != nil {
		if l != nil && l.ID == fw.lesson.ID {
			return
		}
		for _, d := range fw.dirs() {
			fw.w.Remove(d)
		}
	}
	fw.lesson, fw.known = l, map[string]string{}
	if l == nil {
		return
	}
	fw.rewatch()
	work := fw.engine.WorkDir(l)
	for _, name := range l.Meta.Files {
		p := filepath.Join(work, name)
		if data, err := os.ReadFile(p); err == nil {
			fw.known[p] = hashOf(data)
		}
	}
}

// wrote records our own write, so its event isn't reported back, and makes
// sure a folder the write created is watched.
func (fw *fileWatcher) wrote(path, hash string) {
	fw.mu.Lock()
	defer fw.mu.Unlock()
	fw.known[path] = hash
	if fw.w != nil && fw.lesson != nil {
		fw.rewatch()
	}
}

func (fw *fileWatcher) close() {
	if fw.w != nil {
		fw.w.Close()
	}
}

func (fw *fileWatcher) loop() {
	for {
		select {
		case ev, ok := <-fw.w.Events:
			if !ok {
				return
			}
			fw.event(ev)
		case _, ok := <-fw.w.Errors:
			if !ok {
				return
			}
		}
	}
}

func (fw *fileWatcher) event(ev fsnotify.Event) {
	fw.mu.Lock()
	defer fw.mu.Unlock()
	l := fw.lesson
	if l == nil {
		return
	}
	work := fw.engine.WorkDir(l)
	if ev.Name == work || ev.Name == filepath.Dir(work) { // a folder in the chain appeared or was replaced
		fw.rewatch()
		for _, name := range l.Meta.Files {
			fw.later(l, filepath.Join(work, name), name)
		}
		return
	}
	if filepath.Dir(ev.Name) != work {
		return
	}
	name := filepath.Base(ev.Name)
	if slices.Contains(l.Meta.Files, name) {
		fw.later(l, ev.Name, name)
	}
}

// later reports a file once it settles: editors write in several steps.
// Hold fw.mu.
func (fw *fileWatcher) later(l *course.Lesson, path, name string) {
	if t := fw.pending[path]; t != nil {
		t.Stop()
	}
	fw.pending[path] = time.AfterFunc(60*time.Millisecond, func() {
		data, err := os.ReadFile(path)
		if err != nil {
			return // gone for now (Reset, a rename-over save); the next event brings it
		}
		h := hashOf(data)
		fw.mu.Lock()
		if fw.lesson == nil || fw.lesson.ID != l.ID || fw.known[path] == h {
			fw.mu.Unlock()
			return
		}
		fw.known[path] = h
		fw.mu.Unlock()
		fw.notify(FileEvent{Lesson: l.ID, File: File{Name: name, Content: string(data), Hash: h, OnDisk: true}})
	})
}
