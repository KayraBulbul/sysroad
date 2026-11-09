package course

import (
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"slices"
	"sort"
	"strconv"
	"strings"
	"time"

	"gopkg.in/yaml.v3"
)

// CMake lessons (build: cmake) grade a CMake project you write. The engine
// configures and builds it with your CMakeLists.txt, then checks it four ways:
//
//   - the project's shape, read back from CMake's File API: which targets
//     exist, what they link and how they're compiled (targets:, variants:)
//   - the lesson's doctest tests, linked against your library targets
//     (tests_link:), so a library that hides its headers fails to build them
//   - your own CTest tests (ctest:) and `cmake --install` (install:)
//   - checks.yaml, run against the program targets you built
//
// The sanitizers still apply: the engine passes them in CMAKE_CXX_FLAGS.

// CMakeSpec is a CMake lesson's cmake.yaml.
type CMakeSpec struct {
	// Program is the target Run shows and checks.yaml runs by default.
	Program string `yaml:"program"`
	// TestsLink are the targets the lesson's doctest tests link against.
	TestsLink []string       `yaml:"tests_link"`
	Targets   []TargetSpec   `yaml:"targets"`
	Variants  []CMakeVariant `yaml:"variants"`
	CTest     *CTestSpec     `yaml:"ctest"`
	// Install lists files `cmake --install` must put under the prefix.
	Install []string `yaml:"install"`
}

// TargetSpec is what one target must look like.
type TargetSpec struct {
	Name  string   `yaml:"name"`
	Type  string   `yaml:"type"`  // executable, library, static or shared
	Links []string `yaml:"links"` // targets it must link
	Flags []string `yaml:"flags"` // compile flags it must have
	// NotFlags are compile flags it mustn't have.
	NotFlags []string `yaml:"not_flags"`
	// LinkFlags are flags its link step must have (executables).
	LinkFlags []string `yaml:"link_flags"`
	Std       int      `yaml:"std"` // the C++ standard it must ask for, e.g. 20
}

// CMakeVariant configures the project again with other cache variables
// (-D...) and checks the targets there. Nothing is built.
type CMakeVariant struct {
	Name    string            `yaml:"name"`
	Define  map[string]string `yaml:"define"`
	Config  string            `yaml:"config"` // the build type it must end up with
	Targets []TargetSpec      `yaml:"targets"`
	Hidden  bool              `yaml:"hidden"`
}

// CTestSpec grades your own tests: ctest must pass, with at least Min
// tests, and some test must set each of Properties (e.g. WILL_FAIL).
type CTestSpec struct {
	Min        int      `yaml:"min"`
	Properties []string `yaml:"properties"`
}

// CMake reads the lesson's cmake.yaml.
func (l *Lesson) CMake() (*CMakeSpec, error) {
	s, ok := l.read("cmake.yaml")
	if !ok {
		return nil, fmt.Errorf("%s: build: cmake needs a cmake.yaml", l.ID)
	}
	var spec CMakeSpec
	if err := yaml.Unmarshal([]byte(s), &spec); err != nil {
		return nil, fmt.Errorf("%s/cmake.yaml: %w", l.ID, err)
	}
	return &spec, nil
}

// CMakeTools finds cmake and ctest.
func CMakeTools() (cmake, ctest string, err error) {
	cmake, err = exec.LookPath("cmake")
	if err != nil {
		return "", "", errors.New(cmakeMissing)
	}
	ctest = filepath.Join(filepath.Dir(cmake), "ctest")
	if _, err := os.Stat(ctest); err != nil {
		if ctest, err = exec.LookPath("ctest"); err != nil {
			return "", "", errors.New(cmakeMissing)
		}
	}
	return cmake, ctest, nil
}

const cmakeMissing = "cmake isn't installed. Omarchy: sudo pacman -S cmake ninja · Mac: brew install cmake ninja"

// generator is $CMAKE_GENERATOR if you set one, else Ninja when it's
// installed (faster, and the output is quieter), else Makefiles, which
// every Mac and Linux box has.
// Multi-config generators (Ninja Multi-Config, Xcode, Visual Studio) are
// skipped: grading builds and reads back one Debug configuration.
func generator() string {
	if g := os.Getenv("CMAKE_GENERATOR"); g != "" && !multiConfig(g) {
		return g
	}
	if _, err := exec.LookPath("ninja"); err == nil {
		return "Ninja"
	}
	return "Unix Makefiles"
}

func multiConfig(g string) bool {
	return strings.Contains(g, "Multi-Config") || g == "Xcode" || strings.HasPrefix(g, "Visual Studio")
}

// minCMake is the oldest CMake the grading works with: cmake_language(DEFER)
// needs 3.19, and the lessons ask for 3.20.
var minCMake = [2]int{3, 20}

var cmakeVersionRE = regexp.MustCompile(`cmake version (\d+)\.(\d+)`)

// checkCMakeVersion returns an error for a CMake older than minCMake.
func checkCMakeVersion(ctx context.Context, cmake string) error {
	cctx, cancel := context.WithTimeout(ctx, 5*time.Second)
	defer cancel()
	out, err := exec.CommandContext(cctx, cmake, "--version").Output()
	if err != nil {
		return fmt.Errorf("cmake --version failed: %v", err)
	}
	m := cmakeVersionRE.FindStringSubmatch(string(out))
	if m == nil {
		return nil // an unusual build; let it try
	}
	major, _ := strconv.Atoi(m[1])
	minor, _ := strconv.Atoi(m[2])
	if major < minCMake[0] || (major == minCMake[0] && minor < minCMake[1]) {
		return fmt.Errorf("%s is too old: these lessons need CMake %d.%d or newer. Omarchy: sudo pacman -S cmake · Mac: brew upgrade cmake",
			firstLine(string(out)), minCMake[0], minCMake[1])
	}
	return nil
}

// projectInclude runs at the end of each project() call (through
// CMAKE_PROJECT_INCLUDE). The first time, it queues the grader to run once
// the top-level CMakeLists.txt is done, when all your targets exist.
// (A deferred call expands its variables when it runs, so the grader's
// path is written in literally.)
func projectInclude(grader string) string {
	return `get_property(_sysroad_queued GLOBAL PROPERTY SYSROAD_GRADER_QUEUED)
if(NOT _sysroad_queued AND CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set_property(GLOBAL PROPERTY SYSROAD_GRADER_QUEUED TRUE)
  cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL include "` + filepath.ToSlash(grader) + `")
endif()
`
}

// graderCMake adds the lesson's test programs: one with the visible tests
// (Run) and one with them all (Submit), linked against your targets.
func graderCMake(link, visible, all []string, doctestDir, doctestMain, testsDir, binDir string) string {
	q := func(s string) string { return `"` + strings.ReplaceAll(filepath.ToSlash(s), `"`, `\"`) + `"` }
	var b strings.Builder
	// Without the targets to link, there are no tests to build; the
	// project checks say what's missing.
	for _, t := range link {
		fmt.Fprintf(&b, "if(NOT TARGET %s)\n  return()\nendif()\n", t)
	}
	for _, set := range []struct {
		name  string
		files []string
	}{{"sysroad_tests_run", visible}, {"sysroad_tests_submit", all}} {
		var srcs []string
		for _, f := range set.files {
			srcs = append(srcs, q(f))
		}
		fmt.Fprintf(&b, "add_executable(%s EXCLUDE_FROM_ALL %s %s)\n", set.name, strings.Join(srcs, " "), q(doctestMain))
		if len(link) > 0 {
			fmt.Fprintf(&b, "target_link_libraries(%s PRIVATE %s)\n", set.name, strings.Join(link, " "))
		}
		fmt.Fprintf(&b, "target_include_directories(%s SYSTEM PRIVATE %s %s)\n", set.name, q(doctestDir), q(testsDir))
		fmt.Fprintf(&b, "target_compile_options(%s PRIVATE -w -include %s)\n", set.name, q(filepath.Join(doctestDir, "sysroad_doctest.hpp")))
		fmt.Fprintf(&b, "target_compile_features(%s PRIVATE cxx_std_20)\n", set.name)
		fmt.Fprintf(&b, "set_target_properties(%s PROPERTIES RUNTIME_OUTPUT_DIRECTORY %s)\n", set.name, q(binDir))
	}
	return b.String()
}

// doctestPackage is a stand-in for the doctest repository, so a lesson's
// FetchContent_Declare(doctest ...) works offline: the engine points
// FETCHCONTENT_SOURCE_DIR_DOCTEST at it. It has the same layout and
// targets as the real one: doctest::doctest and doctest::doctest_with_main.
func (e *Engine) doctestPackage() (string, error) {
	h := sha256.Sum256(doctestHeader)
	dir := filepath.Join(e.CacheDir, "doctest-pkg", hex.EncodeToString(h[:8]))
	if _, err := os.Stat(filepath.Join(dir, "CMakeLists.txt")); err == nil {
		return dir, nil
	}
	tmp := fmt.Sprintf("%s.%d.tmp", dir, os.Getpid())
	os.RemoveAll(tmp)
	files := map[string][]byte{
		"doctest/doctest.h": doctestHeader,
		"doctest/parts/doctest_main.cpp": []byte(
			"#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN\n#include \"doctest/doctest.h\"\n"),
		"CMakeLists.txt": []byte(`cmake_minimum_required(VERSION 3.14)
project(doctest VERSION 2.4.11 LANGUAGES CXX)
add_library(doctest INTERFACE)
add_library(doctest::doctest ALIAS doctest)
target_include_directories(doctest INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/doctest")
target_compile_features(doctest INTERFACE cxx_std_11)
add_library(doctest_with_main STATIC EXCLUDE_FROM_ALL doctest/parts/doctest_main.cpp)
add_library(doctest::doctest_with_main ALIAS doctest_with_main)
target_link_libraries(doctest_with_main PUBLIC doctest)
`),
	}
	for name, data := range files {
		p := filepath.Join(tmp, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			return "", err
		}
		if err := os.WriteFile(p, data, 0o644); err != nil {
			return "", err
		}
	}
	if err := os.Rename(tmp, dir); err != nil {
		os.RemoveAll(tmp)
		if _, serr := os.Stat(filepath.Join(dir, "CMakeLists.txt")); serr == nil {
			return dir, nil // another process won the race
		}
		return "", err
	}
	return dir, nil
}

// writeIfChanged leaves the file alone when it already has this content,
// so CMake doesn't see a newer file and configure again.
func writeIfChanged(p string, data []byte) error {
	if old, err := os.ReadFile(p); err == nil && bytes.Equal(old, data) {
		return nil
	}
	if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
		return err
	}
	return os.WriteFile(p, data, 0o644)
}

// cmakeFlags are the engine's flags for CMAKE_CXX_FLAGS: debug info, no
// optimisation, the sanitizers. The C++ standard is left to your project.
func (e *Engine) cmakeFlags(ctx context.Context, cxx string) (string, string) {
	flags, note := e.flags(ctx, cxx)
	var keep []string
	for _, f := range flags {
		if !strings.HasPrefix(f, "-std=") {
			keep = append(keep, f)
		}
	}
	return strings.Join(keep, " "), note
}

func (e *Engine) runCMake(ctx context.Context, l *Lesson, mode Mode) (*Result, error) {
	spec, err := l.CMake()
	if err != nil {
		return nil, err
	}
	work, err := e.Ensure(l)
	if err != nil {
		return nil, err
	}
	cmake, ctest, err := CMakeTools()
	if err != nil {
		return &Result{Stage: "compile", Summary: "✗ cmake isn't installed", Output: err.Error() + "\n"}, nil
	}
	if err := checkCMakeVersion(ctx, cmake); err != nil {
		return &Result{Stage: "compile", Summary: "✗ cmake is too old", Output: err.Error() + "\n"}, nil
	}
	cxx, err := Compiler()
	if err != nil {
		return &Result{Stage: "compile", Summary: "no compiler", Output: err.Error()}, nil
	}
	build := e.buildDir(l)
	bdir := filepath.Join(build, "cmake")
	testsDir := filepath.Join(build, "tests")
	binDir := filepath.Join(build, "bin")
	graderDir := filepath.Join(build, "sysroad")

	tests, err := l.tests()
	if err != nil {
		return nil, err
	}
	var visible, all []string
	for _, name := range sortedKeys(tests) {
		p := filepath.Join(testsDir, name)
		if err := writeIfChanged(p, tests[name]); err != nil {
			return nil, err
		}
		if strings.HasSuffix(name, ".cpp") {
			all = append(all, p)
			if !strings.HasPrefix(name, "hidden") {
				visible = append(visible, p)
			}
		}
	}
	hasTests := len(all) > 0

	cxxFlags, sanNote := e.cmakeFlags(ctx, cxx)
	pkg, err := e.doctestPackage()
	if err != nil {
		return nil, err
	}
	define := []string{
		"-DCMAKE_BUILD_TYPE=Debug",
		"-DCMAKE_CXX_COMPILER=" + cxx,
		"-DCMAKE_CXX_FLAGS=" + cxxFlags,
		"-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
		"-DFETCHCONTENT_SOURCE_DIR_DOCTEST=" + pkg,
		"-DFETCHCONTENT_FULLY_DISCONNECTED=ON",
	}
	if hasTests {
		inc, obj, err := e.doctest(ctx, cxx)
		if err != nil {
			return &Result{Stage: "compile", Summary: "✗ the test framework doesn't build", Output: err.Error()}, nil
		}
		grader := graderCMake(spec.TestsLink, visible, all, inc, obj, testsDir, binDir)
		if err := writeIfChanged(filepath.Join(graderDir, "grader.cmake"), []byte(grader)); err != nil {
			return nil, err
		}
		if err := writeIfChanged(filepath.Join(graderDir, "project_include.cmake"), []byte(projectInclude(filepath.Join(graderDir, "grader.cmake")))); err != nil {
			return nil, err
		}
		define = append(define, "-DCMAKE_PROJECT_INCLUDE="+filepath.Join(graderDir, "project_include.cmake"))
	}

	var out strings.Builder
	if sanNote != "" {
		out.WriteString(dim(sanNote) + "\n\n")
	}
	fail := func(summary, text string) *Result {
		return &Result{Stage: "compile", Summary: summary, Output: out.String() + text}
	}

	// Configure.
	text, err := e.configure(ctx, cmake, work, bdir, define)
	if err != nil {
		return fail("✗ CMake can't configure your project", text+"\n"), nil
	}
	if text != "" {
		out.WriteString(text + "\n\n")
	}
	// Build everything your project builds.
	text, err = cmakeBuild(ctx, cmake, work, bdir, "")
	if err != nil {
		return fail("✗ doesn't build", text+"\n"), nil
	}
	if text != "" {
		out.WriteString(text + "\n\n")
	}

	r := &Result{}
	headed := false
	model, err := readCodeModel(bdir)
	if err != nil {
		return fail("✗ couldn't read the project back from CMake", err.Error()+"\n"), nil
	}
	project := func(ok bool, what string, problems []string) {
		r.Total++
		if ok {
			r.Passed++
			return
		}
		r.Stage = "project"
		if !headed {
			headed = true
			out.WriteString(section("Project", ""))
		}
		out.WriteString(red("✗ ") + what + "\n")
		for _, p := range problems {
			out.WriteString("  " + p + "\n")
		}
	}
	for _, t := range spec.Targets {
		problems := model.check(t, cxx)
		project(len(problems) == 0, "target "+t.Name, problems)
	}
	for i, v := range spec.Variants {
		if mode == ModeRun && v.Hidden {
			continue
		}
		if mode == ModeRun && r.Stage != "" {
			break
		}
		problems := e.checkVariant(ctx, cmake, cxx, work, filepath.Join(build, fmt.Sprintf("variant-%d", i)), pkg, v)
		name := v.Name
		if v.Hidden {
			name += dim(" (hidden)")
		}
		project(len(problems) == 0, name, problems)
	}
	if r.Stage != "" {
		out.WriteString("\n")
	}

	g := graded{r: r, progs: map[string]string{}}
	for name, t := range model.targets {
		if t.Type == "EXECUTABLE" && len(t.Artifacts) > 0 {
			g.progs[name] = model.path(t.Artifacts[0].Path)
		}
	}
	g.prog = g.progs[spec.Program]
	if spec.Program != "" && g.prog == "" && r.Stage == "" {
		project(false, "target "+spec.Program, []string{"There's no program target named " + spec.Program + ". add_executable(" + spec.Program + " ...) makes one."})
		out.WriteString("\n")
	}

	for _, t := range spec.TestsLink {
		if model.targets[t] == nil {
			hasTests = false
			if !slices.ContainsFunc(spec.Targets, func(s TargetSpec) bool { return s.Name == t }) {
				project(false, "target "+t, []string{"The lesson's tests link a target named " + t + ", and your project doesn't make one."})
				out.WriteString("\n")
			}
		}
	}
	// The tests are added at the end of the first project() call; without
	// one, there's nothing to hook them onto.
	if hasTests && model.targets["sysroad_tests_run"] == nil {
		hasTests = false
		project(false, "project()", []string{"CMakeLists.txt has no project(...) call. It goes right after cmake_minimum_required."})
		out.WriteString("\n")
	}
	stopped := mode == ModeRun && r.Stage != ""
	if hasTests && !stopped {
		target := "sysroad_tests_" + string(mode)
		text, err := cmakeBuild(ctx, cmake, work, bdir, target)
		if err != nil {
			msg := strings.ReplaceAll(text, testsDir+string(filepath.Separator), "tests/")
			return fail("✗ the tests can't build against your code", msg+"\n"+
				dim("Check that the functions the lesson asks for keep their exact names and signatures,\nand that the tests can see the headers of "+strings.Join(spec.TestsLink, ", ")+".")+"\n"), nil
		}
		g.testBin = filepath.Join(binDir, target)
		if t := model.targets[target]; t != nil && len(t.Artifacts) > 0 {
			g.testBin = model.path(t.Artifacts[0].Path) // where CMake really put it
		}
	}
	if spec.CTest != nil && !stopped {
		problems, text := e.runCTest(ctx, ctest, work, bdir, *spec.CTest)
		if text != "" && len(problems) > 0 {
			out.WriteString(text + "\n")
		}
		project(len(problems) == 0, "your CTest tests", problems)
		if len(problems) > 0 {
			out.WriteString("\n")
		}
	}
	stopped = mode == ModeRun && r.Stage != ""
	if len(spec.Install) > 0 && !stopped {
		problems := installCheck(ctx, cmake, bdir, spec.Install)
		project(len(problems) == 0, "cmake --install", problems)
		if len(problems) > 0 {
			out.WriteString("\n")
		}
	}
	return e.grade(ctx, l, mode, &out, g), nil
}

// configure runs cmake -S work -B bdir. A build folder made with another
// generator (Ninja was installed since) is started over.
func (e *Engine) configure(ctx context.Context, cmake, work, bdir string, define []string) (string, error) {
	gen := generator()
	if cache, err := os.ReadFile(filepath.Join(bdir, "CMakeCache.txt")); err == nil &&
		!bytes.Contains(cache, []byte("\nCMAKE_GENERATOR:INTERNAL="+gen+"\n")) {
		os.RemoveAll(bdir)
	}
	query := filepath.Join(bdir, ".cmake", "api", "v1", "query", "codemodel-v2")
	if err := writeIfChanged(query, nil); err != nil {
		return "", err
	}
	cctx, cancel := context.WithTimeout(ctx, compileTimeout)
	defer cancel()
	args := append([]string{"-S", work, "-B", bdir, "-G", gen}, define...)
	cmd := exec.CommandContext(cctx, cmake, args...)
	cmd.Dir = work
	res, err := cmd.CombinedOutput()
	if cctx.Err() != nil {
		return "CMake took over a minute and was stopped.", cctx.Err()
	}
	return cleanCMake(string(res), work, bdir), err
}

func cmakeBuild(ctx context.Context, cmake, work, bdir, target string) (string, error) {
	cctx, cancel := context.WithTimeout(ctx, 2*compileTimeout)
	defer cancel()
	args := []string{"--build", bdir, "--parallel"}
	if target != "" {
		args = append(args, "--target", target)
	}
	cmd := exec.CommandContext(cctx, cmake, args...)
	cmd.Dir = work
	res, err := cmd.CombinedOutput()
	if cctx.Err() != nil {
		return "The build took over two minutes and was stopped.", cctx.Err()
	}
	return cleanCMake(string(res), work, bdir), err
}

var (
	cmakeProgressRE = regexp.MustCompile(`^(\[\d+/\d+\] |\[ *\d+%\] |g?make(\[\d+\])?: |ninja: |FAILED: |-- |Consolidate compiler generated dependencies)`)
	cmakeCommandRE  = regexp.MustCompile(`^(: && )?\S*(clang\+\+|c\+\+|g\+\+|cmake|ar|ranlib)\s`)
)

// cleanCMake keeps the messages from CMake and the compiler and drops the
// progress lines, the compiler command lines and the build folder's path.
func cleanCMake(s, work, bdir string) string {
	sep := string(filepath.Separator)
	var keep []string
	lines := strings.Split(s, "\n")
	for i := 0; i < len(lines); i++ {
		line := lines[i]
		plain := stripANSI(line)
		// The engine's own -D settings that your project doesn't use.
		if strings.HasPrefix(plain, "CMake Warning") && i+1 < len(lines) && strings.Contains(lines[i+1], "Manually-specified variables were not used") {
			for i+1 < len(lines) && (lines[i+1] == "" || strings.HasPrefix(lines[i+1], " ")) {
				i++
			}
			continue
		}
		if strings.HasPrefix(plain, "This warning is for project developers.") {
			continue
		}
		if cmakeProgressRE.MatchString(plain) || cmakeCommandRE.MatchString(plain) ||
			strings.HasPrefix(plain, "Build files have been written") {
			continue
		}
		line = strings.ReplaceAll(line, bdir+sep, "build"+sep)
		line = strings.ReplaceAll(line, work+sep, "")
		keep = append(keep, line)
	}
	// No runs of blank lines, none at either end.
	var tidy []string
	for _, l := range keep {
		if strings.TrimSpace(l) == "" && (len(tidy) == 0 || strings.TrimSpace(tidy[len(tidy)-1]) == "") {
			continue
		}
		tidy = append(tidy, l)
	}
	return strings.TrimSpace(strings.Join(tidy, "\n"))
}

// ---- reading the project back (CMake's File API) ----

type apiTarget struct {
	Name      string `json:"name"`
	ID        string `json:"id"`
	Type      string `json:"type"`
	Artifacts []struct {
		Path string `json:"path"`
	} `json:"artifacts"`
	CompileGroups []struct {
		Fragments []struct {
			Fragment string `json:"fragment"`
		} `json:"compileCommandFragments"`
		Standard *struct {
			Standard string `json:"standard"`
		} `json:"languageStandard"`
	} `json:"compileGroups"`
	Dependencies []struct {
		ID string `json:"id"`
	} `json:"dependencies"`
	Link *struct {
		Fragments []struct {
			Fragment string `json:"fragment"`
			Role     string `json:"role"`
		} `json:"commandFragments"`
	} `json:"link"`
}

type codeModel struct {
	bdir    string
	config  string
	targets map[string]*apiTarget
	names   map[string]string // id → name
}

func (m *codeModel) path(p string) string {
	if filepath.IsAbs(p) {
		return p
	}
	return filepath.Join(m.bdir, filepath.FromSlash(p))
}

func readCodeModel(bdir string) (*codeModel, error) {
	reply := filepath.Join(bdir, ".cmake", "api", "v1", "reply")
	idx, _ := filepath.Glob(filepath.Join(reply, "index-*.json"))
	if len(idx) == 0 {
		return nil, errors.New("CMake wrote no File API reply (needs CMake 3.14 or newer)")
	}
	sort.Strings(idx)
	var index struct {
		Reply map[string]struct {
			JSONFile string `json:"jsonFile"`
		} `json:"reply"`
	}
	if err := readJSON(idx[len(idx)-1], &index); err != nil {
		return nil, err
	}
	cm, ok := index.Reply["codemodel-v2"]
	if !ok {
		return nil, errors.New("CMake's reply has no codemodel")
	}
	var model struct {
		Configurations []struct {
			Name    string `json:"name"`
			Targets []struct {
				JSONFile string `json:"jsonFile"`
			} `json:"targets"`
		} `json:"configurations"`
	}
	if err := readJSON(filepath.Join(reply, cm.JSONFile), &model); err != nil {
		return nil, err
	}
	m := &codeModel{bdir: bdir, targets: map[string]*apiTarget{}, names: map[string]string{}}
	if len(model.Configurations) == 0 {
		return m, nil
	}
	c := model.Configurations[0]
	m.config = c.Name
	for _, t := range c.Targets {
		var at apiTarget
		if err := readJSON(filepath.Join(reply, t.JSONFile), &at); err != nil {
			return nil, err
		}
		m.targets[at.Name] = &at
		m.names[at.ID] = at.Name
	}
	return m, nil
}

func readJSON(p string, v any) error {
	data, err := os.ReadFile(p)
	if err != nil {
		return err
	}
	return json.Unmarshal(data, v)
}

var typeWords = map[string]string{
	"EXECUTABLE":        "an executable (add_executable)",
	"STATIC_LIBRARY":    "a static library",
	"SHARED_LIBRARY":    "a shared library",
	"MODULE_LIBRARY":    "a module library",
	"OBJECT_LIBRARY":    "an object library",
	"INTERFACE_LIBRARY": "an interface library",
	"UTILITY":           "a custom target",
}

// check returns what's wrong with a target, or nothing.
func (m *codeModel) check(want TargetSpec, cxx string) []string {
	t := m.targets[want.Name]
	if t == nil {
		hint := "add_library(" + want.Name + " ...)"
		if want.Type == "executable" {
			hint = "add_executable(" + want.Name + " ...)"
		}
		return []string{"Your project has no target named " + want.Name + ". " + hint + " makes one."}
	}
	var problems []string
	if want.Type != "" && !typeMatches(want.Type, t.Type) {
		problems = append(problems, fmt.Sprintf("It should be %s, but it's %s.", wantTypeWords(want.Type), typeWords[t.Type]))
	}
	deps := map[string]bool{}
	for _, d := range t.Dependencies {
		deps[m.names[d.ID]] = true
	}
	for _, lib := range want.Links {
		// An alias like doctest::doctest_with_main shows up under the
		// target's real name.
		_, real, _ := strings.Cut(lib, "::")
		if !deps[lib] && !deps[real] {
			problems = append(problems, fmt.Sprintf("It doesn't link %s: target_link_libraries(%s PRIVATE %s).", lib, want.Name, lib))
		}
	}
	have := map[string]bool{}
	std := 0
	for _, g := range t.CompileGroups {
		for _, f := range g.Fragments {
			for _, tok := range splitFlags(f.Fragment) {
				have[tok] = true
				if n := stdOf(tok); n > std {
					std = n
				}
			}
		}
		if g.Standard != nil {
			if n, err := strconv.Atoi(g.Standard.Standard); err == nil && n > std {
				std = n
			}
		}
	}
	var missing []string
	for _, f := range want.Flags {
		for _, tok := range splitFlags(f) {
			if !have[tok] && !slices.Contains(missing, tok) {
				missing = append(missing, tok)
			}
		}
	}
	if len(missing) > 0 {
		problems = append(problems, "It isn't compiled with "+strings.Join(missing, " ")+".")
	}
	var extra []string
	for _, f := range want.NotFlags {
		for _, tok := range splitFlags(f) {
			if have[tok] && !slices.Contains(extra, tok) {
				extra = append(extra, tok)
			}
		}
	}
	if len(extra) > 0 {
		problems = append(problems, "It's compiled with "+strings.Join(extra, " ")+", and shouldn't be here.")
	}
	if len(want.LinkFlags) > 0 {
		linked := map[string]bool{}
		if t.Link != nil {
			for _, f := range t.Link.Fragments {
				if f.Role == "flags" {
					for _, tok := range splitFlags(f.Fragment) {
						linked[tok] = true
					}
				}
			}
		}
		var lmissing []string
		for _, f := range want.LinkFlags {
			for _, tok := range splitFlags(f) {
				if !linked[tok] && !slices.Contains(lmissing, tok) {
					lmissing = append(lmissing, tok)
				}
			}
		}
		if len(lmissing) > 0 {
			problems = append(problems, "It isn't linked with "+strings.Join(lmissing, " ")+".")
		}
	}
	if want.Std > 0 && std < want.Std && compilerDefaultStd(cxx) < want.Std {
		problems = append(problems, fmt.Sprintf("It isn't built as C++%d: target_compile_features(%s PRIVATE cxx_std_%d).", want.Std, want.Name, want.Std))
	}
	return problems
}

func typeMatches(want, got string) bool {
	switch want {
	case "executable":
		return got == "EXECUTABLE"
	case "library":
		return strings.HasSuffix(got, "_LIBRARY") && got != "INTERFACE_LIBRARY"
	case "static":
		return got == "STATIC_LIBRARY"
	case "shared":
		return got == "SHARED_LIBRARY"
	case "interface":
		return got == "INTERFACE_LIBRARY"
	}
	return strings.EqualFold(want, got)
}

func wantTypeWords(want string) string {
	switch want {
	case "executable":
		return typeWords["EXECUTABLE"]
	case "library":
		return "a library (add_library)"
	case "static":
		return "a static library (add_library(... STATIC ...))"
	case "shared":
		return "a shared library (add_library(... SHARED ...))"
	case "interface":
		return "an interface library (add_library(... INTERFACE))"
	}
	return want
}

// splitFlags splits a command fragment into flags, and -fsanitize=a,b into
// -fsanitize=a and -fsanitize=b, so either spelling matches.
func splitFlags(s string) []string {
	var out []string
	for _, f := range strings.Fields(s) {
		if rest, ok := strings.CutPrefix(f, "-fsanitize="); ok {
			for _, s := range strings.Split(rest, ",") {
				out = append(out, "-fsanitize="+s)
			}
			continue
		}
		out = append(out, f)
	}
	return out
}

var stdRE = regexp.MustCompile(`^-std=(?:c|gnu)\+\+(\d\w)$`)

func stdOf(flag string) int {
	m := stdRE.FindStringSubmatch(flag)
	if m == nil {
		return 0
	}
	switch m[1] {
	case "2a":
		return 20
	case "2b":
		return 23
	case "2c":
		return 26
	}
	n, _ := strconv.Atoi(m[1])
	if n < 50 { // 11, 14, 17, 20, 23, 26; 98 and 03 are older still
		return n
	}
	return 0
}

// compilerDefaultStd is the standard the compiler uses with no -std flag.
// CMake leaves the flag out when that's already new enough.
func compilerDefaultStd(cxx string) int {
	cmd := exec.Command(cxx, "-dM", "-E", "-x", "c++", os.DevNull)
	data, err := cmd.Output()
	if err != nil {
		return 0
	}
	m := regexp.MustCompile(`#define __cplusplus (\d{4})`).FindSubmatch(data)
	if m == nil {
		return 0
	}
	year, _ := strconv.Atoi(string(m[1]))
	switch {
	case year >= 2023:
		return 23
	case year >= 2020:
		return 20
	case year >= 2017:
		return 17
	}
	return 0
}

// checkVariant configures the project again with v's cache variables and
// without the engine's flags, so what it finds is yours.
func (e *Engine) checkVariant(ctx context.Context, cmake, cxx, work, dir, pkg string, v CMakeVariant) []string {
	define := []string{
		"-DCMAKE_CXX_COMPILER=" + cxx,
		"-DFETCHCONTENT_SOURCE_DIR_DOCTEST=" + pkg,
		"-DFETCHCONTENT_FULLY_DISCONNECTED=ON",
	}
	keys := make([]string, 0, len(v.Define))
	for k := range v.Define {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	var shown []string
	for _, k := range keys {
		define = append(define, "-D"+k+"="+v.Define[k])
		shown = append(shown, "-D"+k+"="+v.Define[k])
	}
	if _, ok := v.Define["CMAKE_BUILD_TYPE"]; !ok {
		define = append(define, "-DCMAKE_BUILD_TYPE=Debug")
	}
	where := dim("(configured with " + strings.Join(shown, " ") + ")")
	// Each variant starts clean: a cached value from last time would hide
	// a default the project should set itself.
	os.RemoveAll(dir)
	text, err := e.configure(ctx, cmake, work, dir, define)
	if err != nil {
		return []string{"CMake can't configure it:", text, where}
	}
	model, err := readCodeModel(dir)
	if err != nil {
		return []string{err.Error()}
	}
	var problems []string
	if v.Config != "" && model.config != v.Config {
		got := model.config
		if got == "" {
			got = "none"
		}
		problems = append(problems, fmt.Sprintf("The build type should be %s, but it's %s.", v.Config, got))
	}
	for _, t := range v.Targets {
		for _, p := range model.check(t, cxx) {
			problems = append(problems, t.Name+": "+p)
		}
	}
	if len(problems) > 0 {
		problems = append(problems, where)
	}
	return problems
}

var ctestSummaryRE = regexp.MustCompile(`(\d+)% tests passed, (\d+) tests? failed out of (\d+)`)

// runCTest runs your CTest tests and checks there are enough of them.
func (e *Engine) runCTest(ctx context.Context, ctest, work, bdir string, spec CTestSpec) ([]string, string) {
	listCmd := exec.CommandContext(ctx, ctest, "--test-dir", bdir, "--show-only=json-v1")
	data, err := listCmd.Output()
	if err != nil {
		return []string{"ctest couldn't list your tests: " + firstLine(err.Error())}, ""
	}
	var list struct {
		Tests []struct {
			Name       string `json:"name"`
			Properties []struct {
				Name  string `json:"name"`
				Value any    `json:"value"`
			} `json:"properties"`
		} `json:"tests"`
	}
	if err := json.Unmarshal(data, &list); err != nil {
		return []string{"ctest's test list didn't parse: " + err.Error()}, ""
	}
	var problems []string
	if len(list.Tests) == 0 {
		return []string{"ctest finds no tests. Call enable_testing() in the top-level CMakeLists.txt, then add_test(...)."}, ""
	}
	if len(list.Tests) < spec.Min {
		problems = append(problems, fmt.Sprintf("There are %d tests; the lesson asks for at least %d.", len(list.Tests), spec.Min))
	}
	for _, prop := range spec.Properties {
		found := false
		for _, t := range list.Tests {
			for _, p := range t.Properties {
				if p.Name == prop && truthy(p.Value) {
					found = true
				}
			}
		}
		if !found {
			problems = append(problems, "No test sets "+prop+": set_tests_properties(name PROPERTIES "+prop+" ...).")
		}
	}
	res := e.exec(ctx, ctest, []string{"--test-dir", bdir, "--output-on-failure", "--no-tests=error", "--timeout", "10"}, "", nil, 3*testTimeout)
	text := strings.TrimSpace(res.stdout + res.stderr)
	if res.code != 0 {
		m := ctestSummaryRE.FindStringSubmatch(text)
		if m != nil {
			problems = append(problems, fmt.Sprintf("%s of %s tests failed.", m[2], m[3]))
		} else {
			problems = append(problems, "ctest failed"+res.note+".")
		}
		return problems, tidyCTest(text, work, bdir)
	}
	return problems, ""
}

func truthy(v any) bool {
	switch x := v.(type) {
	case bool:
		return x
	case string:
		up := strings.ToUpper(x)
		return x != "" && up != "0" && up != "OFF" && up != "FALSE" && up != "NO" && !strings.HasSuffix(up, "-NOTFOUND")
	case float64:
		return x != 0
	case []any:
		return len(x) > 0
	}
	return v != nil
}

// tidyCTest keeps ctest's per-test lines and failure output, not its
// banner, with paths shown from the lesson folder.
func tidyCTest(s, work, bdir string) string {
	sep := string(filepath.Separator)
	var keep []string
	for _, line := range strings.Split(s, "\n") {
		if strings.HasPrefix(line, "Test project ") || strings.HasPrefix(line, "Total Test time") ||
			strings.HasPrefix(line, "Internal ctest changing into directory") || line == "Errors while running CTest" {
			continue
		}
		line = strings.ReplaceAll(line, bdir+sep, "build"+sep)
		line = strings.ReplaceAll(line, work+sep, "")
		if strings.TrimSpace(line) == "" && (len(keep) == 0 || strings.TrimSpace(keep[len(keep)-1]) == "") {
			continue
		}
		keep = append(keep, line)
	}
	return strings.TrimSpace(strings.Join(keep, "\n"))
}

// installCheck installs into a temporary prefix and looks for the files.
func installCheck(ctx context.Context, cmake, bdir string, want []string) []string {
	prefix, err := os.MkdirTemp("", "sysroad-install-*")
	if err != nil {
		return []string{err.Error()}
	}
	defer os.RemoveAll(prefix)
	cctx, cancel := context.WithTimeout(ctx, compileTimeout)
	defer cancel()
	out, err := exec.CommandContext(cctx, cmake, "--install", bdir, "--prefix", prefix).CombinedOutput()
	if err != nil {
		return []string{"cmake --install failed:", cleanCMake(string(out), "", bdir)}
	}
	var problems []string
	for _, f := range want {
		st, err := os.Stat(filepath.Join(prefix, filepath.FromSlash(f)))
		switch {
		case err != nil:
			problems = append(problems, "It doesn't install "+f+".")
		case strings.HasPrefix(f, "bin/") && st.Mode()&0o111 == 0:
			problems = append(problems, f+" is installed, but it isn't executable.")
		}
	}
	return problems
}

// cmakeVersion is for the toolchain check.
func cmakeVersion(ctx context.Context) (string, error) {
	cmake, _, err := CMakeTools()
	if err != nil {
		return "", err
	}
	cctx, cancel := context.WithTimeout(ctx, 5*time.Second)
	defer cancel()
	out, err := exec.CommandContext(cctx, cmake, "--version").Output()
	if err != nil {
		return "", err
	}
	return firstLine(string(out)), nil
}
