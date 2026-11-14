package course

import (
	"context"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
	"sync"
)

// runProject builds a project lesson: everything in your project folder,
// checked against the contract, then each lesson's tests in a binary of its
// own, linked with your code compiled once. Run covers this lesson's
// visible tests; Submit every test and check of this and the project's
// earlier lessons that hasn't been retired since.
func (e *Engine) runProject(ctx context.Context, l *Lesson, mode Mode) (*Result, error) {
	work, err := e.Ensure(l)
	if err != nil {
		return nil, err
	}
	if _, err := os.Stat(filepath.Join(work, filepath.FromSlash(l.project.Header))); err != nil {
		return missingHeader(l), nil
	}
	srcs, err := projectSources(work)
	if err != nil {
		return nil, err
	}
	if len(srcs) == 0 {
		return &Result{Stage: "compile", Summary: "✗ your project has no .cpp files yet",
			Output: "Every .cpp file in your project folder is compiled, the program and the tests both. Write the code in one.\n"}, nil
	}
	cxx, err := Compiler()
	if err != nil {
		return &Result{Stage: "compile", Summary: "no compiler", Output: err.Error()}, nil
	}
	// Run and Submit build apart, so one in the sidebar and one in a shell
	// don't trip over each other's files.
	build := filepath.Join(e.buildDir(l), string(mode))
	suites, err := projectSuites(l, mode, filepath.Join(build, "tests"))
	if err != nil {
		return nil, err
	}
	needProg := l.Meta.Run != nil || slices.ContainsFunc(suites, func(s suite) bool { return len(s.checks) > 0 })
	flags, sanNote := e.flags(ctx, cxx)
	progFlags := flags
	if l.Meta.Optimize {
		progFlags = optimizedFlags()
		sanNote = "Run builds this program with -O2 and no sanitizers, so its timings mean something. The tests still use sanitizers."
	}
	prog := filepath.Join(build, programName(l))
	flush := filepath.Join(build, "sysroad_flush.cpp")
	if err := os.MkdirAll(build, 0o755); err != nil {
		return nil, err
	}
	if needProg {
		if err := os.WriteFile(flush, []byte(flushSource), 0o644); err != nil {
			return nil, err
		}
	}

	cctx, cancel := context.WithTimeout(ctx, compileTimeout)
	defer cancel()
	jobs := newPool()
	var progRes compiled
	var contract *Result
	jobs.run(func() {
		progRes = compile(cctx, work, cxx, programArgs(progFlags, srcs, flush, pathIf(needProg, prog)))
	})
	jobs.run(func() { contract = e.checkContract(cctx, cxx, flags, l, work, filepath.Join(build, "contract")) })

	// Your sources compile once for every lesson's tests, with your main()
	// renamed so doctest's runs instead. Warnings are off here; the
	// program build above already reported them.
	objDir := filepath.Join(build, "obj")
	hasTests := slices.ContainsFunc(suites, func(s suite) bool { return len(s.files) > 0 })
	var inc, doctestObj string
	var doctestErr error
	if hasTests {
		inc, doctestObj, doctestErr = e.doctest(cctx, cxx)
	}
	objs := make([]string, len(srcs))
	objRes := make([]compiled, len(srcs))
	testRes := make([][]compiled, len(suites))
	if hasTests && doctestErr == nil {
		testFlags := append(append([]string{}, flags...), "-w", "-I", ".", "-isystem", inc)
		for i, src := range srcs {
			objs[i] = filepath.Join(objDir, src+".o")
			jobs.run(func() {
				objRes[i] = compileObject(cctx, work, cxx, append(slices.Clone(testFlags), "-Dmain=sysroad_student_main"), src, objs[i])
			})
		}
		for si, s := range suites {
			testRes[si] = make([]compiled, len(s.files))
			for fi, f := range s.files {
				jobs.run(func() {
					testRes[si][fi] = compileObject(cctx, work, cxx, append(slices.Clone(testFlags), "-include", filepath.Join(inc, "sysroad_doctest.hpp")), f, f+".o")
				})
			}
		}
	}
	jobs.wait()

	var out strings.Builder
	if sanNote != "" {
		out.WriteString(dim(sanNote) + "\n\n")
	}
	if progRes.err != nil {
		// A blank project has no main() until you write one.
		if needProg && missingMain(string(progRes.out)) {
			return &Result{Stage: "compile", Summary: "✗ your project has no main() yet",
				Output: out.String() + "The program the checks and Run use starts at main(). Write it in a .cpp file in your project folder.\n"}, nil
		}
		return &Result{Stage: "compile", Summary: "✗ doesn't compile", Output: out.String() + string(progRes.out)}, nil
	}
	if contract != nil {
		contract.Output = out.String() + contract.Output
		return contract, nil
	}
	if doctestErr != nil {
		return &Result{Stage: "compile", Summary: "✗ the test framework doesn't build", Output: doctestErr.Error()}, nil
	}
	cantBuild := func(s suite, res compiled) *Result {
		return testsCantBuild(s, string(progRes.out), tidyBuild(string(res.out), objDir, s.dir))
	}
	for _, res := range objRes {
		if res.err != nil {
			return cantBuild(suites[len(suites)-1], res), nil
		}
	}
	for si, s := range suites {
		for _, res := range testRes[si] {
			if res.err != nil {
				return cantBuild(s, res), nil
			}
		}
	}
	links := make([]compiled, len(suites))
	for si, s := range suites {
		if len(s.files) == 0 {
			continue
		}
		suites[si].bin = filepath.Join(build, "tests-"+s.lesson.ID)
		jobs.run(func() {
			args := append(append(slices.Clone(flags), objs...), suiteObjects(s)...)
			links[si] = compile(cctx, work, cxx, append(args, doctestObj, "-o", suites[si].bin))
		})
	}
	jobs.wait()
	for si, s := range suites {
		if links[si].err != nil {
			return cantBuild(s, links[si]), nil
		}
	}
	if w := strings.TrimSpace(string(progRes.out)); w != "" {
		out.WriteString(section("Compiler warnings", "") + w + "\n\n")
	}
	return e.grade(ctx, l, mode, &out, graded{prog: pathIf(needProg, prog), suites: suites, r: &Result{}}), nil
}

// projectSuites writes the tests a project lesson's Run or Submit uses into
// dir, a folder per lesson: for Run the lesson's own visible tests, for
// Submit every test of this and the project's earlier lessons, less what
// the lessons up to this one retire. Each folder gets the helpers as of its
// lesson.
func projectSuites(l *Lesson, mode Mode, dir string) ([]suite, error) {
	if err := os.RemoveAll(dir); err != nil {
		return nil, err
	}
	lessons := []*Lesson{l}
	if mode == ModeSubmit {
		lessons = l.project.upTo(l)
	}
	retired := l.retired()
	var out []suite
	for _, x := range lessons {
		var gone Retire
		all := false
		for _, r := range retired[x.ID] {
			all = all || r.all()
			gone.Tests = append(gone.Tests, r.Tests...)
			gone.Cases = append(gone.Cases, r.Cases...)
			gone.Checks = append(gone.Checks, r.Checks...)
		}
		if all {
			continue
		}
		s := suite{lesson: x, earlier: x != l, dir: filepath.Join(dir, x.ID), exclude: gone.Cases}
		tests, err := x.suiteTests()
		if err != nil {
			return nil, err
		}
		for _, name := range sortedKeys(tests) {
			if (mode == ModeRun && strings.HasPrefix(name, "hidden")) || slices.Contains(gone.Tests, name) {
				continue
			}
			p := filepath.Join(s.dir, filepath.FromSlash(name))
			if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
				return nil, err
			}
			if err := os.WriteFile(p, tests[name], 0o644); err != nil {
				return nil, err
			}
			if isTestFile(name) {
				s.files = append(s.files, p)
			}
		}
		checks, err := x.Checks()
		if err != nil {
			return nil, err
		}
		s.checks = slices.DeleteFunc(checks, func(c Check) bool { return slices.Contains(gone.Checks, c.Name) })
		out = append(out, s)
	}
	return out, nil
}

// missingMain reports whether a program build failed only because nothing
// defines main(): GNU ld and lld on Linux, ld64 on macOS.
func missingMain(out string) bool {
	found := false
	for _, line := range strings.Split(stripANSI(out), "\n") {
		line = strings.TrimSpace(line)
		switch {
		case strings.HasSuffix(line, "undefined reference to `main'"), strings.HasSuffix(line, "undefined symbol: main"),
			line == `"_main", referenced from:`:
			found = true
		case strings.Contains(line, "undefined reference to"), strings.Contains(line, "undefined symbol:"),
			strings.HasSuffix(line, "referenced from:"):
			return false // something else is missing too
		case strings.Contains(line, "error:") && !strings.Contains(line, "linker command failed"):
			return false
		}
	}
	return found
}

func suiteObjects(s suite) []string {
	objs := make([]string, len(s.files))
	for i, f := range s.files {
		objs[i] = f + ".o"
	}
	return objs
}

// compileObject compiles one source file to obj.
func compileObject(ctx context.Context, dir, cxx string, flags []string, src, obj string) compiled {
	if err := os.MkdirAll(filepath.Dir(obj), 0o755); err != nil {
		return compiled{err: err, out: []byte(err.Error())}
	}
	return compile(ctx, dir, cxx, append(flags, "-c", src, "-o", obj))
}

// testsCantBuild reports tests that don't compile or link against your
// code, saying so when they're an earlier lesson's.
func testsCantBuild(s suite, warnings, msg string) *Result {
	summary := "✗ the tests can't build against your code"
	head := ""
	hint := "Check that the functions the lesson asks for keep their exact names and signatures."
	if s.earlier {
		summary = "✗ an earlier lesson's tests can't build against your code"
		head = section("Tests", s.origin())
		hint = "These tests still apply. Check that what they use keeps its name and signature."
	}
	return &Result{Stage: "compile", Summary: summary, Output: head + warnings + msg + "\n" + dim(hint) + "\n"}
}

// tidyBuild shows the copied test files as tests/… and your object files
// by their source's name.
func tidyBuild(s, objDir, testsDir string) string {
	sep := string(filepath.Separator)
	s = strings.ReplaceAll(s, testsDir+sep, "tests/")
	return strings.ReplaceAll(s, objDir+sep, "")
}

// pool runs jobs a few at a time: about one compiler per core.
type pool struct {
	wg  sync.WaitGroup
	sem chan struct{}
}

func newPool() *pool { return &pool{sem: make(chan struct{}, max(runtime.NumCPU(), 2))} }

func (p *pool) run(f func()) {
	p.wg.Add(1)
	go func() {
		defer p.wg.Done()
		p.sem <- struct{}{}
		defer func() { <-p.sem }()
		f()
	}()
}

func (p *pool) wait() { p.wg.Wait() }
