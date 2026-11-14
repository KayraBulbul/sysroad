package gui

import (
	"errors"
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"
	"time"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// openTally opens the service on a fresh data dir with the course
// package's fixture project: tally-01-words, tally-02-lines and
// tally-03-report. Events the service sends to the frontend's file
// listener arrive on the channel.
func openTally(t *testing.T) (*Service, *app.App, chan FileEvent) {
	t.Helper()
	c, err := course.LoadFS(os.DirFS("../course/testdata/tally"))
	if err != nil {
		t.Fatal(err)
	}
	dir := t.TempDir()
	if err := app.Seed(dir); err != nil {
		t.Fatal(err)
	}
	a, err := app.OpenCourse(dir, c)
	if err != nil {
		t.Fatal(err)
	}
	events := make(chan FileEvent, 64)
	s := New(a, func(name string, data any) {
		if ev, ok := data.(FileEvent); ok && name == EvFile {
			events <- ev
		}
	}, func(string) error { return nil })
	t.Cleanup(func() { Stop(s) })
	return s, a, events
}

const words, lines, report = "tally-01-words", "tally-02-lines", "tally-03-report"

func names(files []File) []string {
	var out []string
	for _, f := range files {
		out = append(out, f.Name)
	}
	return out
}

// The project folder starts empty and you make every file: the editor
// shows what's there, header first, and saves go to any file in it.
func TestProjectFilesAreTheFolders(t *testing.T) {
	s, a, _ := openTally(t)
	files, err := s.OpenFiles(words)
	if err != nil || len(files) != 0 {
		t.Fatalf("before the folder exists: %v, %v", names(files), err)
	}

	for _, name := range []string{"src/count.cpp", "main.cpp", "tally.hpp"} {
		f, err := s.CreateFile(words, name)
		if err != nil {
			t.Fatalf("create %s: %v", name, err)
		}
		if f.Name != name || !f.OnDisk || f.Content != "" {
			t.Fatalf("created %+v", f)
		}
	}
	if a.Progress.StatusOf(words) != store.Doing {
		t.Fatalf("making a file should start the lesson, it's %s", a.Progress.StatusOf(words))
	}
	work := a.Engine.WorkDir(a.Lesson(words))
	// A program you built in the folder isn't a file to edit.
	if err := os.WriteFile(filepath.Join(work, "tally"), []byte("\x7fELF\x00\x01"), 0o755); err != nil {
		t.Fatal(err)
	}

	files, err = s.OpenFiles(words)
	if err != nil {
		t.Fatal(err)
	}
	// words.txt is the first lesson's starter data, put in with the folder.
	want := []string{"tally.hpp", "main.cpp", "src/count.cpp", "words.txt"}
	if got := names(files); !slices.Equal(got, want) {
		t.Fatalf("files = %v, want %v", got, want)
	}
	for _, f := range files {
		if f.ReadOnly {
			t.Errorf("%s is read-only in the lesson the project is on", f.Name)
		}
	}

	r, err := s.SaveFile(words, "src/count.cpp", "int x;\n", files[2].Hash)
	if err != nil || r.Conflict {
		t.Fatalf("saving a nested file: %+v %v", r, err)
	}
	if got, _ := os.ReadFile(filepath.Join(work, "src", "count.cpp")); string(got) != "int x;\n" {
		t.Fatalf("src/count.cpp = %q", got)
	}
	// A file deleted outside the app comes back when its buffer saves,
	// folders and all.
	os.RemoveAll(filepath.Join(work, "src"))
	if _, err := s.SaveFile(words, "src/count.cpp", "int y;\n", r.Hash); err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(filepath.Join(work, "src", "count.cpp")); string(got) != "int y;\n" {
		t.Fatalf("src/count.cpp = %q after saving it back", got)
	}
}

// Names you give files stay inside the folder and out of what builds and
// sync leave out, and nothing is overwritten.
func TestProjectFileNamesAreChecked(t *testing.T) {
	s, a, _ := openTally(t)
	if _, err := s.CreateFile(words, "tally.hpp"); err != nil {
		t.Fatal(err)
	}
	if _, err := s.CreateFile(words, "main.cpp"); err != nil {
		t.Fatal(err)
	}
	work := a.Engine.WorkDir(a.Lesson(words))
	outside := t.TempDir()
	if err := os.Symlink(outside, filepath.Join(work, "elsewhere")); err != nil {
		t.Fatal(err)
	}

	bad := map[string]string{
		"empty":            "",
		"blank":            "   ",
		"absolute":         "/tmp/x.cpp",
		"up":               "../x.cpp",
		"up inside":        "src/../../x.cpp",
		"dot file":         ".clangd",
		"dot folder":       ".cache/x.cpp",
		"folder only":      "src/",
		"empty folder":     "src//x.cpp",
		"build folder":     "build/x.cpp",
		"object file":      "count.o",
		"a.out":            "a.out",
		"temp file":        "x.tmp",
		"backslash":        `src\x.cpp`,
		"through a link":   "elsewhere/x.cpp",
		"existing file":    "main.cpp",
		"existing header":  "tally.hpp",
		"nested in a file": "main.cpp/x.cpp",
	}
	for why, name := range bad {
		if _, err := s.CreateFile(words, name); err == nil {
			t.Errorf("create %q (%s) was allowed", name, why)
		}
		if _, err := s.RenameFile(words, "main.cpp", name); err == nil {
			t.Errorf("rename to %q (%s) was allowed", name, why)
		}
	}
	if entries, _ := os.ReadDir(outside); len(entries) > 0 {
		t.Fatal("a file landed outside the project folder")
	}
	if _, err := os.Stat(filepath.Join(work, "main.cpp")); err != nil {
		t.Fatal("a refused rename moved main.cpp")
	}
	if _, err := s.RenameFile(words, "missing.cpp", "x.cpp"); err == nil {
		t.Error("renaming a file that isn't there was allowed")
	}
	if _, err := s.DeleteFile(words, "../course.yaml"); err == nil {
		t.Error("deleting outside the folder was allowed")
	}
}

// Rename keeps the content; delete moves the file aside instead of
// destroying it, as Reset does.
func TestRenameAndDeleteProjectFiles(t *testing.T) {
	s, a, _ := openTally(t)
	f, err := s.CreateFile(words, "count.cpp")
	if err != nil {
		t.Fatal(err)
	}
	if _, err := s.SaveFile(words, "count.cpp", "int n;\n", f.Hash); err != nil {
		t.Fatal(err)
	}
	work := a.Engine.WorkDir(a.Lesson(words))

	moved, err := s.RenameFile(words, "count.cpp", "src/count.cpp")
	if err != nil {
		t.Fatal(err)
	}
	if moved.Name != "src/count.cpp" || moved.Content != "int n;\n" {
		t.Fatalf("renamed: %+v", moved)
	}
	if _, err := os.Stat(filepath.Join(work, "count.cpp")); !errors.Is(err, os.ErrNotExist) {
		t.Fatal("the old name is still there")
	}

	dst, err := s.DeleteFile(words, "src/count.cpp")
	if err != nil {
		t.Fatal(err)
	}
	backups := filepath.Join(a.Dir, ".local", "reset-backup") + string(filepath.Separator)
	if !strings.HasPrefix(dst, backups) || !strings.HasSuffix(dst, filepath.Join("src", "count.cpp")) {
		t.Fatalf("deleted file went to %s", dst)
	}
	if got, _ := os.ReadFile(dst); string(got) != "int n;\n" {
		t.Fatalf("the moved-aside file holds %q", got)
	}
	if files, _ := s.OpenFiles(words); slices.Contains(names(files), "src/count.cpp") {
		t.Fatal("a deleted file is still listed")
	}

	// Deleting again in the same second keeps both copies.
	if _, err := s.CreateFile(words, "src/count.cpp"); err != nil {
		t.Fatal(err)
	}
	again, err := s.DeleteFile(words, "src/count.cpp")
	if err != nil || again == dst {
		t.Fatalf("second delete went to %s (first %s): %v", again, dst, err)
	}
	if got, _ := os.ReadFile(dst); string(got) != "int n;\n" {
		t.Fatal("the second delete overwrote the first's copy")
	}
}

// You may make and delete files in a terminal next to the app: the open
// editor hears of each, in folders made after the watch started too.
func TestWatchSeesProjectFilesComeAndGo(t *testing.T) {
	s, a, events := openTally(t)
	s.Watch(words) // before the folder exists
	if _, err := s.CreateFile(words, "tally.hpp"); err != nil {
		t.Fatal(err)
	}
	work := a.Engine.WorkDir(a.Lesson(words))

	waitFor := func(what string, match func(FileEvent) bool) {
		t.Helper()
		deadline := time.After(3 * time.Second)
		for {
			select {
			case ev := <-events:
				if ev.Lesson == words && match(ev) {
					return
				}
				if ev.Name == "tally.hpp" {
					t.Fatalf("our own create was reported back: %+v", ev)
				}
			case <-deadline:
				t.Fatalf("no event for %s", what)
			}
		}
	}

	if err := os.MkdirAll(filepath.Join(work, "src", "util"), 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(work, "src", "util", "text.cpp"), []byte("// nvim\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	waitFor("a file made in a new folder", func(ev FileEvent) bool {
		return ev.Name == "src/util/text.cpp" && !ev.Removed && ev.Content == "// nvim\n" && !ev.ReadOnly
	})

	if err := os.Remove(filepath.Join(work, "src", "util", "text.cpp")); err != nil {
		t.Fatal(err)
	}
	waitFor("a file removed", func(ev FileEvent) bool { return ev.Name == "src/util/text.cpp" && ev.Removed })

	// Build output and dot files aren't the project's.
	os.WriteFile(filepath.Join(work, "main.o"), []byte("obj"), 0o644)
	os.WriteFile(filepath.Join(work, ".main.cpp.swp"), []byte("swap"), 0o644)
	os.WriteFile(filepath.Join(work, "main.cpp"), []byte("int main() {}\n"), 0o644)
	waitFor("main.cpp", func(ev FileEvent) bool {
		if ev.Name == "main.o" || ev.Name == ".main.cpp.swp" {
			t.Fatalf("reported %s", ev.Name)
		}
		return ev.Name == "main.cpp"
	})
}

// A project's lessons go one at a time: the page says which one the
// project is on, and only that one's files change.
func TestProjectOrderAndCatchUp(t *testing.T) {
	s, a, _ := openTally(t)
	d, err := s.Lesson(words)
	if err != nil {
		t.Fatal(err)
	}
	p := d.Project
	if p == nil || p.ID != "tally" || p.Header != "tally.hpp" || p.State != ProjectCurrent || p.OnID != words {
		t.Fatalf("first lesson: %+v", p)
	}
	if !slices.Equal(p.Contract, []string{"int count_words(const std::string&);"}) {
		t.Fatalf("contract: %q", p.Contract)
	}
	if d, _ := s.Lesson(lines); d.Project.State != ProjectLocked || d.Project.OnID != words || d.Project.OnTitle != "Counting words" {
		t.Fatalf("a lesson the project hasn't reached: %+v", d.Project)
	}
	if d, _ := s.Lesson("tally-aside"); d.Project != nil {
		t.Fatal("a lesson outside the project has project info")
	}
	// The first lesson says which files it puts in the folder with the
	// first one you make, so the empty folder's note can say so.
	if !slices.Equal(p.Starter, []string{"words.txt"}) {
		t.Fatalf("starter files: %v", p.Starter)
	}
	if d, _ := s.Lesson(lines); len(d.Project.Starter) != 0 {
		t.Fatalf("a later lesson puts nothing in the folder, but lists %v", d.Project.Starter)
	}
	if d, _ := s.Lesson(report); !slices.Equal(d.Project.Removes, []string{"src/lines.cpp"}) {
		t.Fatalf("files the reference deletes: %v", d.Project.Removes)
	}
	var order *app.OrderError
	if _, err := s.CreateFile(lines, "x.cpp"); !errors.As(err, &order) {
		t.Fatalf("making a file in a locked lesson: %v", err)
	}

	if _, err := s.CreateFile(words, "mine.cpp"); err != nil {
		t.Fatal(err)
	}
	up, err := s.CatchUp(words)
	if err != nil {
		t.Fatal(err)
	}
	if got, _ := os.ReadFile(filepath.Join(up.Backup, "mine.cpp")); up.Backup == "" || got == nil {
		t.Fatalf("your files should be in the backup %q", up.Backup)
	}
	ref, _ := a.Lesson(words).Solution()
	if len(up.Files) != len(ref) || up.Files[0].Name != "tally.hpp" {
		t.Fatalf("after catching up the folder holds %v", names(up.Files))
	}
	for _, f := range up.Files {
		if f.Content != string(ref[f.Name]) || !f.ReadOnly {
			t.Errorf("%s isn't the read-only reference", f.Name)
		}
	}
	if up.Lesson.Status != store.Skipped || up.Lesson.Project.State != ProjectPassed || up.Lesson.Project.OnID != lines {
		t.Fatalf("caught-up lesson: %s, %+v", up.Lesson.Status, up.Lesson.Project)
	}
	if _, err := s.CatchUp(words); !errors.As(err, &order) {
		t.Fatalf("catching up a passed lesson: %v", err)
	}
	if _, err := s.SaveFile(words, "tally.hpp", "// late\n", up.Files[0].Hash); !errors.As(err, &order) {
		t.Fatalf("saving in a passed lesson: %v", err)
	}
	if d, _ := s.Lesson(lines); d.Project.State != ProjectCurrent {
		t.Fatalf("the next lesson: %+v", d.Project)
	}
	files, err := s.OpenFiles(lines)
	if err != nil || len(files) != len(ref) || files[0].ReadOnly {
		t.Fatalf("the lesson the project is on now: %v %v", files, err)
	}
}
