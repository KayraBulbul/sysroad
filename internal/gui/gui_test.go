package gui

import (
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/KayraBulbul/sysroad/internal/app"
)

// omarchy-theme-set deletes current/theme and moves the next theme into its
// place; the watcher has to keep following the new folder, more than once.
func TestWatchThemeFollowsOmarchySwap(t *testing.T) {
	root := t.TempDir()
	current := filepath.Join(root, "current")
	dir := filepath.Join(current, "theme")
	write := func(d, bg string) {
		os.MkdirAll(d, 0o755)
		os.WriteFile(filepath.Join(d, "colors.toml"), []byte("mode = \"dark\"\nbackground = \""+bg+"\"\nforeground = \"#eeeeee\"\n"), 0o644)
	}
	write(dir, "#111111")

	got := make(chan Theme, 8)
	stop := make(chan struct{})
	defer close(stop)
	if err := WatchTheme(dir, func(th Theme) { got <- th }, stop); err != nil {
		t.Fatal(err)
	}
	swap := func(bg string) {
		next := filepath.Join(current, "next-theme")
		write(next, bg)
		os.RemoveAll(dir)
		if err := os.Rename(next, dir); err != nil {
			t.Fatal(err)
		}
	}
	for _, bg := range []string{"#222222", "#333333"} {
		swap(bg)
		select {
		case th := <-got:
			if th.Colors["background"] != bg {
				t.Fatalf("got background %s, want %s", th.Colors["background"], bg)
			}
		case <-time.After(3 * time.Second):
			t.Fatalf("no theme change after swapping to %s", bg)
		}
	}
}

// A save names the content it started from; if the file changed on disk
// since (the other machine syncing, an edit in nvim), it must not be
// overwritten, and the change on disk is reported to the open editor.
func TestSaveFileNeverClobbersDiskChanges(t *testing.T) {
	dir := t.TempDir()
	if err := app.Seed(dir); err != nil {
		t.Fatal(err)
	}
	a, err := app.Open(dir)
	if err != nil {
		t.Fatal(err)
	}
	events := make(chan FileEvent, 8)
	s := New(a, func(name string, data any) {
		if ev, ok := data.(FileEvent); ok && name == EvFile {
			events <- ev
		}
	}, func(string) error { return nil })
	defer Stop(s)

	const id, name = "cpp-01-echo", "echo.cpp"
	files, err := s.OpenFiles(id)
	if err != nil || len(files) == 0 {
		t.Fatalf("OpenFiles: %v %v", files, err)
	}
	if files[0].OnDisk {
		t.Fatal("an unstarted lesson shouldn't have a folder yet")
	}

	r, err := s.SaveFile(id, name, "// mine\n", files[0].Hash)
	if err != nil || r.Conflict {
		t.Fatalf("first save: %+v %v", r, err)
	}
	path := filepath.Join(a.Engine.WorkDir(a.Lesson(id)), name)

	// Our own save isn't reported back as a change.
	select {
	case ev := <-events:
		t.Fatalf("own save reported as a disk change: %+v", ev)
	case <-time.After(300 * time.Millisecond):
	}

	if err := os.WriteFile(path, []byte("// from nvim\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	select {
	case ev := <-events:
		if ev.Name != name || ev.Content != "// from nvim\n" {
			t.Fatalf("disk change event: %+v", ev)
		}
	case <-time.After(3 * time.Second):
		t.Fatal("no event for an edit made outside the app")
	}

	r, err = s.SaveFile(id, name, "// mine, edited\n", r.Hash)
	if err != nil {
		t.Fatal(err)
	}
	if !r.Conflict || r.Disk == nil || r.Disk.Content != "// from nvim\n" {
		t.Fatalf("stale save wasn't refused: %+v", r)
	}
	if b, _ := os.ReadFile(path); string(b) != "// from nvim\n" {
		t.Fatalf("file was overwritten: %q", b)
	}
}
