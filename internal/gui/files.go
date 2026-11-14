package gui

import (
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"slices"
	"strings"
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
	Name     string `json:"name"`
	Content  string `json:"content"`
	Hash     string `json:"hash"`
	OnDisk   bool   `json:"onDisk"`   // false: starter code, the folder isn't made yet
	ReadOnly bool   `json:"readOnly"` // a file the lesson gives you, like its header
}

// FileEvent reports a lesson file that changed on disk. In a project it
// may also be a file that appeared, or one that went (Removed).
type FileEvent struct {
	Lesson  string `json:"lesson"`
	Removed bool   `json:"removed"`
	File
}

type ResultEvent struct {
	Lesson string `json:"lesson"`
}

func hashOf(b []byte) string {
	h := sha256.Sum256(b)
	return hex.EncodeToString(h[:8])
}

// editable returns the lesson if name is one of the files you edit in it:
// in a project, any file in its folder, while the project is on the lesson.
func (s *Service) editable(id, name string) (*course.Lesson, error) {
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	if !l.HasCode() {
		return nil, errors.New("this lesson has no code to edit")
	}
	if l.Project() != nil {
		if err := checkFileName(name); err != nil {
			return nil, err
		}
		if err := s.app.CheckOrder(l); err != nil {
			return nil, err
		}
		return l, nil
	}
	if !slices.Contains(l.Meta.Files, name) {
		return nil, fmt.Errorf("%q isn't one of this lesson's files", name)
	}
	return l, nil
}

// OpenFiles returns the files you edit in a lesson, then the starter's other
// files (headers it shares with the tests) read-only. A lesson you haven't
// started yet shows its starter code; the folder is made on the first save or
// run, so browsing doesn't litter the data repo. In a project it's every
// text file in the project's folder, the contract header first: none while
// the folder is empty or not made yet. They're read-only unless the project
// is on this lesson.
func (s *Service) OpenFiles(id string) ([]File, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	if !l.HasCode() {
		return nil, nil
	}
	if l.Project() != nil {
		return s.projectFiles(l)
	}
	starter, err := l.Starter()
	if err != nil {
		return nil, err
	}
	work := s.app.Engine.WorkDir(l)
	var out []File
	for _, name := range l.SortedNames(starter) {
		f := File{Name: name, ReadOnly: !slices.Contains(l.Meta.Files, name)}
		if data, err := os.ReadFile(filepath.Join(work, filepath.FromSlash(name))); err == nil {
			f.Content, f.Hash, f.OnDisk = string(data), hashOf(data), true
		} else {
			f.Content, f.Hash = string(starter[name]), hashOf(starter[name])
		}
		out = append(out, f)
	}
	return out, nil
}

// Watch follows a lesson's files on disk (FileEvent), or none for "". It's a
// separate call from OpenFiles so that quick lesson switches, whose
// OpenFiles calls can finish in any order, only watch the one that won.
func (s *Service) Watch(id string) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l := s.app.Lesson(id)
	if l != nil && !l.HasCode() {
		l = nil
	}
	s.files.watch(l)
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
	if l.Project() != nil {
		// Any file in the folder; a file that went (deleted in nvim) comes
		// back, folders and all.
		if path, err = projectPath(work, name); err != nil {
			return nil, err
		}
		if st, err := os.Lstat(path); err == nil && !st.Mode().IsRegular() {
			return nil, fmt.Errorf("%s isn't a file", name)
		}
		if err := os.MkdirAll(filepath.Dir(path), 0o755); err != nil {
			return nil, err
		}
	}
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

// Reset puts the starter code back (in a project, the folder as it was at
// the start of the lesson), moving your files to .local/reset-backup/. It
// returns the backup folder and the new files.
func (s *Service) Reset(id string) (string, []File, error) {
	s.mu.Lock()
	l, err := s.lesson(id)
	var backup string
	if err == nil {
		backup, err = s.app.ResetLesson(l)
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

// Solution is the lesson's reference solution, for the files you edit; in
// a project, the files this lesson's reference adds or changes.
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
	sol, err := l.PeekSolution()
	if err != nil {
		return nil, err
	}
	names := l.SortedNames(sol)
	if p := l.Project(); p != nil {
		names = projectOrder(names, p.Header)
	}
	var out []File
	for _, name := range names {
		out = append(out, File{Name: name, Content: string(sol[name]), Hash: hashOf(sol[name])})
	}
	return out, nil
}

// ---- watching ----

// fileWatcher follows the open lesson's folder. It watches the chain from
// the data folder down (data, work/, work/<lesson>) and re-adds it whenever
// one of those changes, so a folder that appears (first save, a git pull) or
// is replaced (Reset) keeps being followed.
//
// In a project it follows every file in the folder, however deep, and also
// reports files that appear and files that go: you may make, rename or
// delete them in a terminal next to the app.
type fileWatcher struct {
	engine *course.Engine
	notify func(FileEvent)

	mu      sync.Mutex
	w       *fsnotify.Watcher
	lesson  *course.Lesson
	known   map[string]string // path → last hash reported or written
	pending map[string]*time.Timer
	subdirs []string // a project's folders inside its own, as watched now
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

// dirs is every folder to watch: data/, work/, the lesson folder, and each
// subfolder that holds one of the lesson's files (tests/ for
// tests/CMakeLists.txt), or in a project every folder in it. fsnotify isn't
// recursive, and a file is only reported by its own folder's watch.
func (fw *fileWatcher) dirs() []string {
	work := fw.engine.WorkDir(fw.lesson)
	out := []string{fw.engine.DataDir, filepath.Dir(work), work}
	if fw.lesson.Project() != nil {
		return append(out, fw.subdirs...)
	}
	for _, name := range fw.lesson.Meta.Files {
		for d := filepath.Dir(filepath.Join(work, name)); d != work && strings.HasPrefix(d, work+string(filepath.Separator)); d = filepath.Dir(d) {
			if !slices.Contains(out, d) {
				out = append(out, d)
			}
		}
	}
	return out
}

// projectDirs lists the folders inside a project folder, however deep,
// without the ones builds leave out (build/, dot folders).
func projectDirs(work string) []string {
	var out []string
	filepath.WalkDir(work, func(p string, d fs.DirEntry, err error) error {
		switch {
		case err != nil && p == work:
			return fs.SkipAll
		case err != nil || p == work || !d.IsDir():
			return nil
		case course.LeftOut(d.Name(), true):
			return fs.SkipDir
		}
		out = append(out, p)
		return nil
	})
	return out
}

// rewatch (re)adds the chain; a folder that doesn't exist yet is skipped
// until its parent reports it. In a project it first looks for the folders
// in it again, dropping the watches of ones that went. Hold fw.mu.
func (fw *fileWatcher) rewatch() {
	if fw.lesson.Project() != nil {
		now := projectDirs(fw.engine.WorkDir(fw.lesson))
		for _, d := range fw.subdirs {
			if !slices.Contains(now, d) {
				fw.w.Remove(d)
			}
		}
		fw.subdirs = now
	}
	for _, d := range fw.dirs() {
		fw.w.Add(d)
	}
}

// tracked is every file of the lesson the watcher reports, as name → path.
func (fw *fileWatcher) tracked() map[string]string {
	work := fw.engine.WorkDir(fw.lesson)
	names := fw.lesson.Meta.Files
	if fw.lesson.Project() != nil {
		names, _ = fw.engine.WorkFiles(fw.lesson)
	}
	out := map[string]string{}
	for _, name := range names {
		out[name] = filepath.Join(work, filepath.FromSlash(name))
	}
	return out
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
	fw.lesson, fw.known, fw.subdirs = l, map[string]string{}, nil
	if l == nil {
		return
	}
	fw.rewatch()
	for _, p := range fw.tracked() {
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

// moved records our own rename (to a path with this hash) or delete (to
// ""), so neither is reported back.
func (fw *fileWatcher) moved(from, to, hash string) {
	fw.mu.Lock()
	defer fw.mu.Unlock()
	delete(fw.known, from)
	if to != "" {
		fw.known[to] = hash
	}
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
	if slices.Contains(fw.dirs(), ev.Name) { // a watched folder appeared, went or was replaced
		fw.rewatch()
		fw.rescan()
		return
	}
	// Files are named relative to the lesson folder: tests/CMakeLists.txt.
	rel, err := filepath.Rel(work, ev.Name)
	if err != nil {
		return
	}
	name := filepath.ToSlash(rel)
	if l.Project() == nil {
		if slices.Contains(l.Meta.Files, name) {
			fw.later(l, ev.Name, name)
		}
		return
	}
	if checkFileName(name) != nil { // outside the folder, a dot file, build output
		return
	}
	if st, err := os.Lstat(ev.Name); err == nil && st.IsDir() { // a new folder
		fw.rewatch()
		fw.rescan()
		return
	}
	fw.later(l, ev.Name, name)
}

// rescan looks at every file again, after a folder changed: the lesson's
// files, or in a project every file in it now and every one it had. Hold
// fw.mu.
func (fw *fileWatcher) rescan() {
	files := fw.tracked()
	for name, p := range files {
		fw.later(fw.lesson, p, name)
	}
	if fw.lesson.Project() == nil {
		return
	}
	work := fw.engine.WorkDir(fw.lesson)
	for p := range fw.known {
		if rel, err := filepath.Rel(work, p); err == nil {
			if name := filepath.ToSlash(rel); files[name] == "" {
				fw.later(fw.lesson, p, name)
			}
		}
	}
}

// later reports a file once it settles: editors write in several steps.
// Hold fw.mu.
func (fw *fileWatcher) later(l *course.Lesson, path, name string) {
	if t := fw.pending[path]; t != nil {
		t.Stop()
	}
	fw.pending[path] = time.AfterFunc(60*time.Millisecond, func() {
		project := l.Project() != nil
		data, err := os.ReadFile(path)
		fw.mu.Lock()
		if fw.lesson == nil || fw.lesson.ID != l.ID {
			fw.mu.Unlock()
			return
		}
		if err != nil {
			// Gone for now (Reset, a rename-over save); the next event
			// brings it. In a project, a file we'd reported is gone.
			_, had := fw.known[path]
			if !project || !had || !errors.Is(err, fs.ErrNotExist) {
				fw.mu.Unlock()
				return
			}
			delete(fw.known, path)
			fw.mu.Unlock()
			fw.notify(FileEvent{Lesson: l.ID, Removed: true, File: File{Name: name}})
			return
		}
		h := hashOf(data)
		if fw.known[path] == h || (project && !isText(data)) {
			fw.mu.Unlock()
			return
		}
		fw.known[path] = h
		fw.mu.Unlock()
		fw.notify(FileEvent{Lesson: l.ID, File: File{Name: name, Content: string(data), Hash: h, OnDisk: true}})
	})
}
