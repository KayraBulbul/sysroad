package course

import (
	"context"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
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
	checkSolutions(t, c)
}

// checkSolutions runs every code lesson of c: lessons on their own, and
// each project's lessons in order in one folder.
func checkSolutions(t *testing.T, c *Course) {
	cache := t.TempDir()
	// Each lesson runs two compilers at once, so this keeps about one per
	// core: more and the ASan test binaries crowd their 10s limit.
	sem := make(chan struct{}, max(1, runtime.NumCPU()/2))
	for _, l := range c.Lessons() {
		if !l.HasCode() || l.Project() != nil {
			continue
		}
		t.Run(l.ID, func(t *testing.T) {
			// Lessons that time things run one at a time, before the rest
			// start, so nothing slows their measurements.
			if !l.Meta.Optimize {
				t.Parallel()
			}
			sem <- struct{}{}
			defer func() { <-sem }()
			if l.Meta.Build == "cmake" {
				if _, _, err := CMakeTools(); err != nil {
					t.Skip("no cmake")
				}
			}
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
			writeFiles(t, e.WorkDir(l), sol)
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
	for _, p := range c.Projects() {
		t.Run(p.ID, func(t *testing.T) {
			t.Parallel()
			sem <- struct{}{}
			defer func() { <-sem }()
			walkProject(t, &Engine{DataDir: t.TempDir(), CacheDir: cache}, p)
		})
	}
}

// walkProject takes a project's lessons in order in one folder: before a
// lesson's reference is there, Run must fail; with exactly the reference in
// the folder, Submit must pass, earlier lessons' tests included.
func walkProject(t *testing.T, e *Engine, p *Project) {
	for _, l := range p.Lessons {
		walkLesson(t, e, l)
	}
}

func walkLesson(t *testing.T, e *Engine, l *Lesson) {
	t.Helper()
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
	defer cancel()
	r, err := e.Run(ctx, l, ModeRun)
	if err != nil {
		t.Fatal(err)
	}
	if r.OK {
		t.Errorf("%s: the previous reference passes Run — the lesson has nothing to do:\n%s", l.ID, stripANSI(r.Output))
	}
	ref, err := l.Solution()
	if err != nil {
		t.Fatal(err)
	}
	have, _ := e.WorkFiles(l)
	for _, f := range have {
		if _, ok := ref[f]; !ok {
			os.Remove(filepath.Join(e.WorkDir(l), filepath.FromSlash(f)))
		}
	}
	writeFiles(t, e.WorkDir(l), ref)
	r, err = e.Run(ctx, l, ModeSubmit)
	if err != nil {
		t.Fatal(err)
	}
	if !r.OK || r.Total == 0 {
		t.Fatalf("%s: the reference fails Submit: %s\n%s", l.ID, r.Summary, stripANSI(r.Output))
	}
	if err := e.SaveSnapshot(l); err != nil {
		t.Fatal(err)
	}
}

func writeFiles(t *testing.T, dir string, files map[string][]byte) {
	t.Helper()
	for name, data := range files {
		p := filepath.Join(dir, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(p, data, 0o644); err != nil {
			t.Fatal(err)
		}
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
