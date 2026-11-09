package course

import (
	"context"
	"crypto/sha256"
	_ "embed"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"sort"
	"strings"
	"sync"
	"time"
)

//go:embed third_party/doctest.h
var doctestHeader []byte

// Force-included into test files so failures show strings quoted and escaped.
//
//go:embed third_party/sysroad_doctest.hpp
var doctestPrelude []byte

// Mode is what a learner asked for: Run (visible tests, stop at the first
// failure, show a sample run) or Submit (every test; a pass completes it).
type Mode string

const (
	ModeRun    Mode = "run"
	ModeSubmit Mode = "submit"
)

// Result of one Run or Submit. It is saved under .local/runs/ so every
// sysroad window (and the GUI later) shows the latest one, wherever it ran.
type Result struct {
	Lesson  string    `json:"lesson"`
	Mode    Mode      `json:"mode"`
	At      time.Time `json:"at"`
	OK      bool      `json:"ok"`
	Stage   string    `json:"stage"` // compile, tests, checks, check, done
	Summary string    `json:"summary"`
	Output  string    `json:"output"`
	Passed  int       `json:"passed"`
	Total   int       `json:"total"`

	// Interactive is set when Run should hand the terminal to the program.
	Interactive *exec.Cmd `json:"-"`
}

// Engine builds and tests learner code for one data directory.
type Engine struct {
	DataDir  string
	CacheDir string
}

func NewEngine(dataDir string) *Engine {
	cache, err := os.UserCacheDir()
	if err != nil {
		cache = os.TempDir()
	}
	return &Engine{DataDir: dataDir, CacheDir: filepath.Join(cache, "sysroad")}
}

const WorkDirName = "work"

func (e *Engine) WorkDir(l *Lesson) string { return filepath.Join(e.DataDir, WorkDirName, l.ID) }
func (e *Engine) buildDir(l *Lesson) string {
	return filepath.Join(e.DataDir, ".local", "build", l.ID)
}
func (e *Engine) runPath(id string) string {
	return filepath.Join(e.DataDir, ".local", "runs", id+".json")
}

// Ensure creates the lesson's workspace from its starter files. Files that
// already exist are never touched, so it is safe to call at any time.
func (e *Engine) Ensure(l *Lesson) (string, error) {
	dir := e.WorkDir(l)
	if !l.Written || l.Meta.Check != "" {
		return dir, nil
	}
	starter, err := l.Starter()
	if err != nil {
		return "", err
	}
	for name, data := range starter {
		p := filepath.Join(dir, filepath.FromSlash(name))
		if _, err := os.Stat(p); err == nil {
			continue
		}
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			return "", err
		}
		if err := os.WriteFile(p, data, 0o644); err != nil {
			return "", err
		}
	}
	return dir, nil
}

// Reset moves the workspace aside (into .local/reset-backup/) and recreates
// it from the starter files. It returns where the old files went.
func (e *Engine) Reset(l *Lesson) (string, error) {
	dir := e.WorkDir(l)
	backup := ""
	if _, err := os.Stat(dir); err == nil {
		backup = filepath.Join(e.DataDir, ".local", "reset-backup", l.ID+"-"+time.Now().Format("20060102-150405"))
		if err := os.MkdirAll(filepath.Dir(backup), 0o755); err != nil {
			return "", err
		}
		if err := os.Rename(dir, backup); err != nil {
			return "", err
		}
	}
	_, err := e.Ensure(l)
	return backup, err
}

// LastResult is the most recent saved Run/Submit result for a lesson.
func (e *Engine) LastResult(id string) *Result {
	data, err := os.ReadFile(e.runPath(id))
	if err != nil {
		return nil
	}
	var r Result
	if json.Unmarshal(data, &r) != nil {
		return nil
	}
	return &r
}

// ResultModTime lets views notice a result written by another process.
func (e *Engine) ResultModTime(id string) time.Time {
	st, err := os.Stat(e.runPath(id))
	if err != nil {
		return time.Time{}
	}
	return st.ModTime()
}

// Latest is the lesson with the most recently written result, so a sidebar
// can follow runs started from the editor's shell.
func (e *Engine) Latest() (id string, at time.Time) {
	dir := filepath.Join(e.DataDir, ".local", "runs")
	entries, err := os.ReadDir(dir)
	if err != nil {
		return "", time.Time{}
	}
	for _, ent := range entries {
		name := ent.Name()
		if !strings.HasSuffix(name, ".json") {
			continue
		}
		if info, err := ent.Info(); err == nil && info.ModTime().After(at) {
			id, at = strings.TrimSuffix(name, ".json"), info.ModTime()
		}
	}
	return id, at
}

func (e *Engine) save(r *Result) {
	p := e.runPath(r.Lesson)
	if os.MkdirAll(filepath.Dir(p), 0o755) != nil {
		return
	}
	data, _ := json.Marshal(r)
	// Per-process temp name: the sidebar and a shell run may save at once.
	tmp := fmt.Sprintf("%s.%d.tmp", p, os.Getpid())
	if os.WriteFile(tmp, data, 0o644) == nil {
		os.Rename(tmp, p)
	}
}

// ---- toolchain ----

// Compiler is $SYSROAD_CXX, else clang++, else c++.
func Compiler() (string, error) {
	if c := os.Getenv("SYSROAD_CXX"); c != "" {
		return c, nil
	}
	for _, c := range []string{"clang++", "c++"} {
		if p, err := exec.LookPath(c); err == nil {
			return p, nil
		}
	}
	return "", errors.New("no C++ compiler found — install clang (Omarchy: sudo pacman -S clang; Mac: xcode-select --install)")
}

var warnFlags = []string{"-Wall", "-Wextra"}

// optimizedFlags is baseFlags with -O2 and assertions off: what a release
// build would use.
func optimizedFlags() []string {
	out := make([]string, 0, len(baseFlags)+1)
	for _, f := range baseFlags {
		if f == "-O0" {
			f = "-O2"
		}
		out = append(out, f)
	}
	return append(out, "-DNDEBUG")
}

// flushSource is linked into every program. When a sanitizer stops the
// program (a bad access, undefined behaviour, or leaks found at exit) it
// exits without flushing stdio, and what the program printed so far would be
// lost. This flushes it first.
const flushSource = `#include <cstdio>
#if defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(undefined_behavior_sanitizer)
#include <sanitizer/common_interface_defs.h>
namespace {
struct SysroadFlush {
    SysroadFlush() {
        __sanitizer_set_death_callback([] { std::fflush(nullptr); });
    }
} sysroad_flush;
}  // namespace
#endif
#endif
`

func sanitizerEnv() []string {
	env := os.Environ()
	asan := "abort_on_error=0"
	if runtime.GOOS == "linux" {
		asan += ":detect_leaks=1" // LeakSanitizer isn't supported on macOS
	}
	return append(env, "ASAN_OPTIONS="+asan, "UBSAN_OPTIONS=print_stacktrace=1")
}

var doctestOnce sync.Mutex

// doctest writes doctest.h into the cache and compiles its main() once per
// compiler+flags combination, so test builds only compile your code.
func (e *Engine) doctest(ctx context.Context, cxx string) (includeDir, mainObj string, err error) {
	doctestOnce.Lock()
	defer doctestOnce.Unlock()
	ver, _ := exec.CommandContext(ctx, cxx, "--version").Output()
	h := sha256.New()
	h.Write(doctestHeader)
	h.Write(doctestPrelude)
	h.Write([]byte(cxx))
	h.Write(ver)
	flags, _ := e.flags(ctx, cxx)
	h.Write([]byte(strings.Join(flags, " ")))
	key := hex.EncodeToString(h.Sum(nil))[:16]
	dir := filepath.Join(e.CacheDir, "doctest", key)
	header := filepath.Join(dir, "doctest.h")
	obj := filepath.Join(dir, "doctest_main.o")
	if _, err := os.Stat(obj); err == nil {
		return dir, obj, nil
	}
	if err := os.MkdirAll(dir, 0o755); err != nil {
		return "", "", err
	}
	if err := os.WriteFile(header, doctestHeader, 0o644); err != nil {
		return "", "", err
	}
	if err := os.WriteFile(filepath.Join(dir, "sysroad_doctest.hpp"), doctestPrelude, 0o644); err != nil {
		return "", "", err
	}
	src := filepath.Join(dir, fmt.Sprintf("doctest_main.%d.cpp", os.Getpid()))
	defer os.Remove(src)
	if err := os.WriteFile(src, []byte("#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN\n#include \"doctest.h\"\n"), 0o644); err != nil {
		return "", "", err
	}
	// Unique per process: the sidebar and a shell `sysroad run` may race here.
	tmp := fmt.Sprintf("%s.%d.tmp", obj, os.Getpid())
	args := append(append([]string{}, flags...), "-w", "-c", src, "-o", tmp)
	if out, err := exec.CommandContext(ctx, cxx, args...).CombinedOutput(); err != nil {
		return "", "", fmt.Errorf("compiling doctest: %v\n%s", err, out)
	}
	return dir, obj, os.Rename(tmp, obj)
}

// ---- running ----

const (
	compileTimeout = 60 * time.Second
	testTimeout    = 10 * time.Second
	checkTimeout   = 5 * time.Second
	outputLimit    = 64 << 10
)

// Run builds and tests a lesson. The result is also saved for other views.
func (e *Engine) Run(ctx context.Context, l *Lesson, mode Mode) (*Result, error) {
	if !l.Written {
		return nil, fmt.Errorf("%q isn't written yet", l.Title)
	}
	if l.IsQuiz() {
		return nil, fmt.Errorf("%q is a quiz: answer its questions instead", l.Title)
	}
	var r *Result
	var err error
	switch {
	case l.Meta.Check != "":
		r, err = e.builtinCheck(ctx, l, mode)
	case l.Meta.Build == "cmake":
		r, err = e.runCMake(ctx, l, mode)
	default:
		r, err = e.runCode(ctx, l, mode)
	}
	if err != nil {
		return nil, err
	}
	r.Lesson, r.Mode, r.At = l.ID, mode, time.Now()
	e.save(r)
	return r, nil
}

type compiled struct {
	out []byte
	err error
}

func (e *Engine) runCode(ctx context.Context, l *Lesson, mode Mode) (*Result, error) {
	work, err := e.Ensure(l)
	if err != nil {
		return nil, err
	}
	build := e.buildDir(l)
	if err := os.MkdirAll(build, 0o755); err != nil {
		return nil, err
	}
	srcs, err := sources(work)
	if err != nil {
		return nil, err
	}
	if len(srcs) == 0 {
		return &Result{Stage: "compile", Summary: "no .cpp files in " + work}, nil
	}
	cxx, err := Compiler()
	if err != nil {
		return &Result{Stage: "compile", Summary: "no compiler", Output: err.Error()}, nil
	}
	checks, err := l.Checks()
	if err != nil {
		return nil, err
	}
	tests, err := l.tests()
	if err != nil {
		return nil, err
	}
	var testFiles []string
	for _, name := range sortedKeys(tests) {
		if mode == ModeRun && strings.HasPrefix(name, "hidden") {
			continue
		}
		p := filepath.Join(build, "tests", name)
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			return nil, err
		}
		if err := os.WriteFile(p, tests[name], 0o644); err != nil {
			return nil, err
		}
		// Headers in tests/ are shared helpers the test files include.
		if strings.HasSuffix(name, ".cpp") {
			testFiles = append(testFiles, p)
		}
	}
	needProg := len(checks) > 0 || l.Meta.Run != nil
	flags, sanNote := e.flags(ctx, cxx)
	// Lessons that time things build the program optimised and without
	// sanitizers, which would otherwise swamp the measurements. The tests
	// keep the sanitizers.
	progFlags := flags
	if l.Meta.Optimize {
		progFlags = optimizedFlags()
		sanNote = "Run builds this program with -O2 and no sanitizers, so its timings mean something. The tests still use sanitizers."
	}

	cctx, cancel := context.WithTimeout(ctx, compileTimeout)
	defer cancel()
	prog := filepath.Join(build, "prog")
	testBin := filepath.Join(build, "tests-"+string(mode))
	var progRes, testRes compiled
	var wg sync.WaitGroup

	flush := filepath.Join(build, "sysroad_flush.cpp")
	if needProg {
		if err := os.WriteFile(flush, []byte(flushSource), 0o644); err != nil {
			return nil, err
		}
	}

	// The program build carries the warnings you see. Without a program
	// to build, a syntax-only pass shows them instead.
	wg.Add(1)
	go func() {
		defer wg.Done()
		args := append(append(append([]string{}, progFlags...), warnFlags...), "-I", ".")
		if needProg {
			args = append(append(args, srcs...), flush, "-o", prog)
		} else {
			args = append(append(args, "-fsyntax-only"), srcs...)
		}
		progRes = compile(cctx, work, cxx, args)
	}()
	if len(testFiles) > 0 {
		wg.Add(1)
		go func() {
			defer wg.Done()
			inc, obj, err := e.doctest(cctx, cxx)
			if err != nil {
				testRes = compiled{err: err}
				return
			}
			// Your main() is renamed so doctest's main runs the tests. Warnings
			// are off here; the program build above already reported them.
			args := append(append([]string{}, flags...), "-w", "-Dmain=sysroad_student_main", "-I", ".", "-isystem", inc,
				"-include", filepath.Join(inc, "sysroad_doctest.hpp"))
			args = append(append(append(args, srcs...), testFiles...), obj, "-o", testBin)
			testRes = compile(cctx, work, cxx, args)
		}()
	}
	wg.Wait()

	var out strings.Builder
	if sanNote != "" {
		out.WriteString(dim(sanNote) + "\n\n")
	}
	if progRes.err != nil {
		return &Result{Stage: "compile", Summary: "✗ doesn't compile", Output: out.String() + string(progRes.out)}, nil
	}
	if testRes.err != nil {
		// The test files are copied into the build folder; show them as tests/….
		msg := strings.ReplaceAll(string(testRes.out), filepath.Join(build, "tests")+string(filepath.Separator), "tests/")
		if msg == "" {
			msg = testRes.err.Error()
		}
		return &Result{Stage: "compile", Summary: "✗ the tests can't build against your code",
			Output: string(progRes.out) + msg + "\n" + dim("Check that the functions the lesson asks for keep their exact names and signatures.") + "\n"}, nil
	}
	if w := strings.TrimSpace(string(progRes.out)); w != "" {
		out.WriteString(w + "\n\n")
	}

	return e.grade(ctx, l, mode, &out, graded{prog: pathIf(needProg, prog), testBin: pathIf(len(testFiles) > 0, testBin), r: &Result{}}), nil
}

func pathIf(ok bool, p string) string {
	if ok {
		return p
	}
	return ""
}

// graded is a built lesson, ready for the sample run, the tests and the checks.
type graded struct {
	prog    string            // what Run shows and checks.yaml runs; "" if none
	progs   map[string]string // CMake: each program target, for checks with program:
	testBin string            // the doctest binary; "" if the lesson has no tests
	r       *Result           // results so far (a CMake project's own checks)
}

// grade runs a built lesson: the sample run, the unit tests, then the
// program checks, and sums it all up. out already holds what came before.
func (e *Engine) grade(ctx context.Context, l *Lesson, mode Mode, out *strings.Builder, g graded) *Result {
	r := g.r
	work, testsDir := e.WorkDir(l), filepath.Join(e.buildDir(l), "tests")
	allOK := r.Stage == ""
	stopped := !allOK && mode == ModeRun
	// Sample run first, like seeing your program's output before the tests.
	if mode == ModeRun && l.Meta.Run != nil && g.prog != "" && !stopped {
		s := l.Meta.Run
		if s.Interactive {
			cmd := exec.Command(g.prog, s.Args...)
			cmd.Dir = work
			cmd.Env = sanitizerEnv()
			r.Interactive = cmd
		} else {
			res := e.exec(ctx, g.prog, s.Args, s.Stdin, s.Files, checkTimeout)
			out.WriteString(dim("$ "+shellLine(progName(l, ""), s.Args, s.Stdin)) + "\n")
			out.WriteString(res.stdout)
			if res.stdout != "" && !strings.HasSuffix(res.stdout, "\n") {
				out.WriteString(dim("⏎ (no newline at end)") + "\n")
			}
			if res.stderr != "" {
				out.WriteString(red(tidySanitizer(res.stderr, work, testsDir)))
			}
			out.WriteString(dim(fmt.Sprintf("exit %d%s", res.code, res.note)) + "\n\n")
		}
	}

	if g.testBin != "" && !stopped {
		args := []string{"--no-version", "--no-intro", "--no-path-filenames", "--force-colors"}
		if mode == ModeRun {
			args = append(args, "--abort-after=1")
		}
		res := e.exec(ctx, g.testBin, args, "", nil, testTimeout)
		passed, total := doctestCounts(res.stdout + res.stderr)
		r.Passed += passed
		r.Total += total
		if res.code != 0 {
			allOK = false
			r.Stage = "tests"
			out.WriteString(cleanDoctest(tidySanitizer(res.stdout+"\n"+res.stderr, work, testsDir)) + "\n")
			if res.note != "" {
				out.WriteString(red(res.note) + "\n")
			}
		}
	}
	checks, err := l.Checks()
	if err != nil {
		allOK, r.Stage = false, "checks"
		out.WriteString(red(err.Error()) + "\n")
	}
	// Submit runs the program checks even after a failed unit test, so you
	// see everything that's left in one go.
	if (allOK || mode == ModeSubmit) && len(checks) > 0 {
		if !allOK {
			out.WriteString("\n")
		}
		ok, passed, total, text := e.runChecks(ctx, l, g, checks, mode)
		r.Passed += passed
		r.Total += total
		out.WriteString(text)
		if !ok {
			allOK = false
			r.Stage = "checks"
		}
	}
	r.Output = strings.TrimRight(out.String(), "\n") + "\n"
	r.OK = allOK
	switch {
	case allOK && mode == ModeSubmit:
		r.Stage, r.Summary = "done", fmt.Sprintf("✓ all %d tests passed — lesson complete", r.Total)
	case allOK:
		r.Stage, r.Summary = "done", fmt.Sprintf("✓ %d visible tests pass — submit when ready", r.Total)
	case r.Stage == "tests" && r.Total == 0:
		r.Summary = "✗ the tests crashed — the report above says where"
	default:
		r.Summary = fmt.Sprintf("✗ %d/%d passed", r.Passed, r.Total)
	}
	return r
}

// cleanDoctest drops doctest's noise lines.
func cleanDoctest(s string) string {
	var keep []string
	for _, l := range strings.Split(strings.TrimSpace(s), "\n") {
		plain := strings.TrimSpace(stripANSI(l))
		if strings.Contains(plain, "Aborting - too many failed asserts") ||
			strings.HasPrefix(plain, "[doctest]") ||
			(plain != "" && strings.Trim(plain, "=") == "") {
			continue
		}
		keep = append(keep, l)
	}
	return strings.TrimSpace(strings.Join(keep, "\n"))
}

func compile(ctx context.Context, dir, cxx string, args []string) compiled {
	cmd := exec.CommandContext(ctx, cxx, args...)
	cmd.Dir = dir
	out, err := cmd.CombinedOutput()
	if ctx.Err() != nil {
		return compiled{out: out, err: fmt.Errorf("compile timed out")}
	}
	return compiled{out: out, err: err}
}

func sources(dir string) ([]string, error) {
	entries, err := os.ReadDir(dir)
	if err != nil {
		return nil, err
	}
	var srcs []string
	for _, e := range entries {
		n := e.Name()
		if !e.IsDir() && (strings.HasSuffix(n, ".cpp") || strings.HasSuffix(n, ".cc")) {
			srcs = append(srcs, n)
		}
	}
	sort.Strings(srcs)
	return srcs, nil
}

func sortedKeys(m map[string][]byte) []string {
	keys := make([]string, 0, len(m))
	for k := range m {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	return keys
}

var doctestCasesRE = regexp.MustCompile(`test cases:\s*(\d+)\s*\|\s*(\d+) passed`)

func doctestCounts(s string) (passed, total int) {
	m := doctestCasesRE.FindStringSubmatch(stripANSI(s))
	if m == nil {
		return 0, 0
	}
	fmt.Sscan(m[1], &total)
	fmt.Sscan(m[2], &passed)
	return passed, total
}

func (e *Engine) runChecks(ctx context.Context, l *Lesson, g graded, checks []Check, mode Mode) (ok bool, passed, total int, text string) {
	var b strings.Builder
	ok = true
	shown, unbuilt := 0, 0
	for _, c := range checks {
		if mode == ModeRun && c.Hidden {
			continue
		}
		total++
		prog := g.prog
		if c.Program != "" {
			prog = g.progs[c.Program]
		}
		if prog == "" {
			// The project checks above already say the program is missing.
			ok = false
			unbuilt++
			continue
		}
		res := e.exec(ctx, prog, c.Args, c.Stdin, c.Files, checkTimeout)
		var problems []string
		if c.Stdout != nil && res.stdout != *c.Stdout {
			problems = append(problems, "stdout")
		}
		wantExit := 0
		if c.Exit != nil {
			wantExit = *c.Exit
		}
		if res.code != wantExit {
			problems = append(problems, "exit")
		}
		if len(problems) == 0 {
			passed++
			continue
		}
		ok = false
		if shown < 3 {
			shown++
			name := c.Name
			if c.Hidden {
				name += dim(" (hidden)")
			}
			b.WriteString(red("✗ ") + name + "\n")
			b.WriteString(dim("  $ "+shellLine(progName(l, c.Program), c.Args, c.Stdin)) + "\n")
			if c.Stdout != nil && res.stdout != *c.Stdout {
				b.WriteString("  expected stdout: " + green(visible(*c.Stdout)) + "\n")
				b.WriteString("  your stdout:     " + red(visible(res.stdout)) + "\n")
			}
			if res.code != wantExit {
				b.WriteString(fmt.Sprintf("  expected exit %d, got %d%s\n", wantExit, res.code, res.note))
			}
			if s := strings.TrimSpace(tidySanitizer(res.stderr, e.WorkDir(l), filepath.Join(e.buildDir(l), "tests"))); s != "" {
				b.WriteString(dim("  stderr:") + "\n" + indent(s, "    ") + "\n")
			}
		}
		if mode == ModeRun {
			break // like boot.dev's Run: stop at the first failure
		}
	}
	if more := total - passed - shown - unbuilt; !ok && more > 0 {
		b.WriteString(dim(fmt.Sprintf("… and %d more failing", more)) + "\n")
	}
	if unbuilt > 0 {
		what := fmt.Sprintf("%d program checks wait", unbuilt)
		if unbuilt == 1 {
			what = "1 program check waits"
		}
		b.WriteString(dim(what+" for programs your project doesn't build yet") + "\n")
	}
	return ok, passed, total, b.String()
}

// progName is how a check shows the program it runs: the CMake target, or
// else the lesson's first file without its extension.
func progName(l *Lesson, program string) string {
	switch {
	case program != "":
		return "./" + program
	case l.Meta.Build == "cmake":
		if spec, err := l.CMake(); err == nil && spec.Program != "" {
			return "./" + spec.Program
		}
	case len(l.Meta.Files) > 0:
		return "./" + strings.TrimSuffix(filepath.Base(l.Meta.Files[0]), filepath.Ext(l.Meta.Files[0]))
	}
	return "./prog"
}

// shellLine renders an invocation like `$ ./echo -n hi < input`.
func shellLine(name string, args []string, stdin string) string {
	parts := []string{name}
	for _, a := range args {
		parts = append(parts, quote(a))
	}
	line := strings.Join(parts, " ")
	if stdin != "" {
		line = "printf " + quote(stdin) + " | " + line
	}
	return line
}

func quote(s string) string {
	if s != "" && !strings.ContainsAny(s, " \t\n'\"\\$*?|&;<>()") {
		return s
	}
	return "'" + strings.ReplaceAll(strings.ReplaceAll(s, "'", `'\''`), "\n", `\n`) + "'"
}

// visible shows invisible characters so output mismatches are obvious.
func visible(s string) string {
	r := strings.NewReplacer("\n", "\\n", "\t", "\\t", "\r", "\\r")
	if s == "" {
		return `""`
	}
	return `"` + r.Replace(s) + `"`
}

func indent(s, pre string) string { return pre + strings.ReplaceAll(s, "\n", "\n"+pre) }

// Palette colours only (0–15) so output follows the terminal theme.
func red(s string) string   { return "\x1b[31m" + s + "\x1b[39m" }
func green(s string) string { return "\x1b[32m" + s + "\x1b[39m" }
func dim(s string) string   { return "\x1b[90m" + s + "\x1b[39m" }

var ansiRE = regexp.MustCompile(`\x1b\[[0-9;]*[A-Za-z]`)

func stripANSI(s string) string { return ansiRE.ReplaceAllString(s, "") }

// ---- built-in checks ----

func (e *Engine) builtinCheck(ctx context.Context, l *Lesson, mode Mode) (*Result, error) {
	switch l.Meta.Check {
	case "toolchain":
		return e.checkToolchain(ctx)
	}
	return nil, fmt.Errorf("unknown check %q", l.Meta.Check)
}

func (e *Engine) checkToolchain(ctx context.Context) (*Result, error) {
	var b strings.Builder
	r := &Result{OK: true}
	item := func(ok, required bool, what, detail string) {
		mark := green("✓ ")
		switch {
		case ok:
		case required:
			mark, r.OK = red("✗ "), false
		default:
			mark = "\x1b[33m! \x1b[39m"
		}
		if required {
			r.Total++
			if ok {
				r.Passed++
			}
		}
		b.WriteString(mark + what)
		if detail != "" {
			b.WriteString(dim("  " + detail))
		}
		b.WriteString("\n")
	}
	cxx, err := Compiler()
	fallback := ""
	if err != nil {
		item(false, true, "C++ compiler", err.Error())
	} else {
		ver, _ := exec.CommandContext(ctx, cxx, "--version").Output()
		first := strings.SplitN(string(ver), "\n", 2)[0]
		item(strings.Contains(strings.ToLower(first), "clang"), true, "clang++", first)

		_, _, choice := e.sanitizers(ctx, cxx, true) // always re-probe here
		for i, row := range choice.Rows {
			// The full set is what lessons want; a weaker set is a fallback.
			item(row.OK, i == len(choice.Rows)-1, "C++20 with "+row.Name, row.Detail)
		}
		if choice.Index > 0 {
			fallback = sanitizerSets[choice.Index].Name
		}
		_, _, derr := e.doctest(ctx, cxx)
		item(derr == nil, true, "test framework (doctest) builds", firstLine(errString(derr)))
	}
	// CMake is only needed from phase 3 on, so it's a warning here.
	if v, err := cmakeVersion(ctx); err == nil {
		item(true, false, "cmake (phase 3 on)", v+" · generator: "+generator())
	} else {
		item(false, false, "cmake (phase 3 on)", err.Error())
	}
	dbg := "gdb"
	if runtime.GOOS == "darwin" {
		dbg = "lldb"
	}
	if p, err := exec.LookPath(dbg); err == nil {
		item(true, true, "debugger: "+dbg, p)
	} else {
		item(false, true, "debugger: "+dbg, "not found")
	}
	r.Stage = "check"
	switch {
	case r.OK && fallback != "":
		r.Stage, r.Summary = "done", "✓ ready, using "+fallback+" on this machine"
		if runtime.GOOS == "darwin" {
			b.WriteString("\n" + dim("Lessons still work, without memory-bug detection. This is a known bug: AddressSanitizer\n"+
				"from Xcode 26.3 or older deadlocks at startup on macOS 26.4+. Update the Command Line Tools to\n"+
				"26.4 or later (System Settings → General → Software Update, or `softwareupdate --list`), then\n"+
				"submit this check again. sysroad re-tests on its own when the compiler version changes.") + "\n")
		} else {
			b.WriteString("\n" + dim("Lessons still work, without AddressSanitizer. Submit this check again after updating clang.") + "\n")
		}
	case r.OK:
		r.Stage, r.Summary = "done", fmt.Sprintf("✓ toolchain ready (%d/%d)", r.Passed, r.Total)
	default:
		r.Summary = fmt.Sprintf("✗ %d/%d checks passed", r.Passed, r.Total)
		if runtime.GOOS == "darwin" {
			b.WriteString(dim("\nMac: xcode-select --install gives you clang++ and lldb.") + "\n")
		} else {
			b.WriteString(dim("\nOmarchy: sudo pacman -S clang gdb") + "\n")
		}
	}
	r.Output = b.String()
	return r, nil
}

func firstLine(s string) string { return strings.SplitN(strings.TrimSpace(s), "\n", 2)[0] }

func errString(err error) string {
	if err == nil {
		return ""
	}
	return err.Error()
}
