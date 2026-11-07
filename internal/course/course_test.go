package course

import (
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"testing"
	"time"
)

func TestCourseLoads(t *testing.T) {
	c, err := Load()
	if err != nil {
		t.Fatal(err)
	}
	if err := c.Validate(); err != nil {
		t.Fatal(err)
	}
	written := 0
	for _, l := range c.Lessons() {
		if l.Written {
			written++
			if l.Body == "" {
				t.Errorf("%s: empty lesson body", l.ID)
			}
		}
	}
	if written < 5 {
		t.Fatalf("only %d written lessons", written)
	}
}

func needCompiler(t *testing.T) {
	t.Helper()
	if _, err := Compiler(); err != nil {
		t.Skip("no C++ compiler")
	}
	if _, err := exec.LookPath("clang++"); err != nil && os.Getenv("SYSROAD_CXX") == "" {
		t.Skip("no clang++")
	}
}

// Every written code lesson: the starter must fail Run (there's work to do)
// and the solution must pass Submit.
func TestLessonsSolutionsPassStartersFail(t *testing.T) {
	needCompiler(t)
	c, err := Load()
	if err != nil {
		t.Fatal(err)
	}
	cache := t.TempDir()
	for _, l := range c.Lessons() {
		if !l.HasCode() {
			continue
		}
		t.Run(l.ID, func(t *testing.T) {
			e := &Engine{DataDir: t.TempDir(), CacheDir: cache}
			ctx, cancel := context.WithTimeout(context.Background(), 2*time.Minute)
			defer cancel()

			r, err := e.Run(ctx, l, ModeRun)
			if err != nil {
				t.Fatal(err)
			}
			if r.OK {
				t.Errorf("starter passes Run — the lesson has nothing to do:\n%s", r.Output)
			}

			sol, err := l.Solution()
			if err != nil {
				t.Fatal(err)
			}
			for name, data := range sol {
				if err := os.WriteFile(filepath.Join(e.WorkDir(l), name), data, 0o644); err != nil {
					t.Fatal(err)
				}
			}
			r, err = e.Run(ctx, l, ModeSubmit)
			if err != nil {
				t.Fatal(err)
			}
			if !r.OK {
				t.Fatalf("solution fails Submit: %s\n%s", r.Summary, stripANSI(r.Output))
			}
			if r.Total == 0 {
				t.Fatal("submit ran no tests")
			}
			if got := e.LastResult(l.ID); got == nil || !got.OK {
				t.Fatal("result wasn't saved")
			}
		})
	}
}

func TestResetKeepsABackup(t *testing.T) {
	c, _ := Load()
	l := c.Lesson("setup-lessons")
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	dir, err := e.Ensure(l)
	if err != nil {
		t.Fatal(err)
	}
	p := filepath.Join(dir, "add.cpp")
	os.WriteFile(p, []byte("mine"), 0o644)
	if _, err := e.Ensure(l); err != nil {
		t.Fatal(err)
	}
	if data, _ := os.ReadFile(p); string(data) != "mine" {
		t.Fatal("Ensure must not overwrite your files")
	}
	backup, err := e.Reset(l)
	if err != nil {
		t.Fatal(err)
	}
	if data, _ := os.ReadFile(filepath.Join(backup, "add.cpp")); string(data) != "mine" {
		t.Fatal("backup should hold your old file")
	}
	if data, _ := os.ReadFile(p); string(data) == "mine" {
		t.Fatal("reset should restore the starter")
	}
}

func TestLevelSpans(t *testing.T) {
	for xp := 0; xp < 2000; xp += 7 {
		lvl, into, span := Level(xp)
		if into < 0 || into >= span || lvl < 1 {
			t.Fatalf("Level(%d) = %d,%d,%d", xp, lvl, into, span)
		}
	}
}

// A hung program that also left a child running must still be killed on
// time; with pipes, the child holding stdout open made Wait block.
func TestExecTimeoutKillsTheWholeGroup(t *testing.T) {
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	start := time.Now()
	r := e.exec(context.Background(), "/bin/sh", []string{"-c", "sleep 30 & echo started; sleep 30"}, "", nil, time.Second)
	if took := time.Since(start); took > 5*time.Second {
		t.Fatalf("exec took %s after a 1s timeout", took)
	}
	if !r.timedOut || r.stdout != "started\n" {
		t.Fatalf("timedOut=%v stdout=%q", r.timedOut, r.stdout)
	}
}

func TestExecPassesStdinArgsAndFiles(t *testing.T) {
	e := &Engine{DataDir: t.TempDir(), CacheDir: t.TempDir()}
	r := e.exec(context.Background(), "/bin/sh", []string{"-c", `cat in.txt; cat; echo "$0"`, "arg0"}, "from stdin\n", map[string]string{"in.txt": "from file\n"}, 5*time.Second)
	if r.code != 0 || r.stdout != "from file\nfrom stdin\narg0\n" {
		t.Fatalf("code=%d stdout=%q stderr=%q", r.code, r.stdout, r.stderr)
	}
}
