package course

import (
	"errors"
	"fmt"
	"regexp"
	"slices"
	"strconv"
	"strings"
)

// Project is a run of lessons that build one program in one folder,
// work/<id>/. The folder starts empty (or with the first lesson's starter
// data files) and holds only your code: each lesson states a narrow
// contract its tests use, and the rest is your design.
type Project struct {
	ID      string
	Header  string    // the contract header, e.g. kite.hpp, relative to the folder
	Lessons []*Lesson // in course order; they needn't be adjacent
}

// Contract is what one lesson adds to its project's contract: the types and
// functions its tests use, declared in the project's header.
type Contract struct {
	Header    string         `yaml:"header"`
	Types     []string       `yaml:"types"`
	Functions []ContractFunc `yaml:"functions"`
	// Drops removes items an earlier lesson added that no longer apply.
	Drops []string `yaml:"drops"`
}

// ContractFunc is a function the tests call, with its C++ function type,
// e.g. "std::string(const Editor&)".
type ContractFunc struct {
	Name string `yaml:"name"`
	Sig  string `yaml:"sig"`
}

// Retire names an earlier project lesson's tests that this lesson's
// contract change makes invalid. With only Lesson set, all of them go.
type Retire struct {
	Lesson string   `yaml:"lesson"`
	Tests  []string `yaml:"tests"`  // whole test files, as named in its tests/
	Cases  []string `yaml:"cases"`  // doctest test-case name patterns (* and ?)
	Checks []string `yaml:"checks"` // checks.yaml check names
}

func (r Retire) all() bool { return len(r.Tests)+len(r.Cases)+len(r.Checks) == 0 }

// Project is the project the lesson belongs to, or nil.
func (l *Lesson) Project() *Project { return l.project }

// Projects returns every project, in the order their first lessons come.
func (c *Course) Projects() []*Project { return c.projects }

// Project looks a project up by ID.
func (c *Course) Project(id string) *Project {
	for _, p := range c.projects {
		if p.ID == id {
			return p
		}
	}
	return nil
}

// linkProjects gathers the lessons of each project, in course order.
func (c *Course) linkProjects() {
	for _, l := range c.Lessons() {
		id := l.Meta.Project
		if id == "" {
			continue
		}
		p := c.Project(id)
		if p == nil {
			p = &Project{ID: id}
			c.projects = append(c.projects, p)
		}
		if p.Header == "" {
			p.Header = l.Meta.Contract.Header
		}
		p.Lessons = append(p.Lessons, l)
		l.project = p
	}
}

func (p *Project) index(l *Lesson) int { return slices.Index(p.Lessons, l) }

// prev is the project lesson before l: nil for the first, or outside a project.
func (p *Project) prev(l *Lesson) *Lesson {
	if p == nil {
		return nil
	}
	if i := p.index(l); i > 0 {
		return p.Lessons[i-1]
	}
	return nil
}

// upTo is the project's lessons from the first through l.
func (p *Project) upTo(l *Lesson) []*Lesson { return p.Lessons[:p.index(l)+1] }

// ContractItem is one type or function in force in a project.
type ContractItem struct {
	Name string
	Sig  string // a function's C++ function type; "" for a type
}

func (it ContractItem) IsType() bool { return it.Sig == "" }

// Decl is the item written the way you'd declare it in the header.
func (it ContractItem) Decl() string {
	if it.IsType() {
		return "struct " + it.Name + " { … };"
	}
	ret, params, quals, err := splitSig(it.Sig)
	if err != nil {
		return it.Name + ": " + it.Sig
	}
	return ret + " " + it.Name + "(" + params + ")" + quals + ";"
}

// Contract is the project's contract in force at this lesson: what the
// project's lessons up to here added, a later item replacing an earlier one
// of the same name, without what they dropped. Nil outside a project.
func (l *Lesson) Contract() []ContractItem {
	if l.project == nil {
		return nil
	}
	var items []ContractItem
	put := func(it ContractItem) {
		if i := slices.IndexFunc(items, func(x ContractItem) bool { return x.Name == it.Name }); i >= 0 {
			items[i] = it
		} else {
			items = append(items, it)
		}
	}
	for _, x := range l.project.upTo(l) {
		c := x.Meta.Contract
		items = slices.DeleteFunc(items, func(it ContractItem) bool { return slices.Contains(c.Drops, it.Name) })
		for _, t := range c.Types {
			put(ContractItem{Name: t})
		}
		for _, f := range c.Functions {
			put(ContractItem{Name: f.Name, Sig: f.Sig})
		}
	}
	return items
}

// retired gathers what the project's lessons up to l retire, by the lesson
// whose tests they retire.
func (l *Lesson) retired() map[string][]Retire {
	out := map[string][]Retire{}
	for _, x := range l.project.upTo(l) {
		for _, r := range x.Meta.Retires {
			out[r.Lesson] = append(out[r.Lesson], r)
		}
	}
	return out
}

// splitSig splits a C++ function type such as "std::string(const Editor&)
// noexcept" into its return type, parameters and trailing qualifiers.
func splitSig(sig string) (ret, params, quals string, err error) {
	s := strings.TrimSpace(sig)
	if rest, ok := strings.CutSuffix(s, "noexcept"); ok {
		s, quals = strings.TrimSpace(rest), " noexcept"
	}
	if !strings.HasSuffix(s, ")") {
		return "", "", "", fmt.Errorf("sig %q isn't a function type like \"int(const std::string&)\"", sig)
	}
	depth := 0
	for i := len(s) - 1; i >= 0; i-- {
		switch s[i] {
		case ')':
			depth++
		case '(':
			depth--
			if depth == 0 {
				ret = strings.TrimSpace(s[:i])
				if ret == "" {
					return "", "", "", fmt.Errorf("sig %q has no return type", sig)
				}
				return ret, s[i+1 : len(s)-1], quals, nil
			}
		}
	}
	return "", "", "", fmt.Errorf("sig %q has unbalanced brackets", sig)
}

var cppName = regexp.MustCompile(`^(::)?[A-Za-z_]\w*(::[A-Za-z_]\w*)*$`)

var testCaseRE = regexp.MustCompile(`TEST_CASE(?:_FIXTURE\s*\(\s*\w+\s*,|\s*\()\s*("(?:[^"\\]|\\.)*")`)

// testCaseNames lists the doctest test-case names in a lesson's test files.
func testCaseNames(tests map[string][]byte) []string {
	var out []string
	for _, data := range tests {
		for _, m := range testCaseRE.FindAllSubmatch(data, -1) {
			if name, err := strconv.Unquote(string(m[1])); err == nil {
				out = append(out, name)
			}
		}
	}
	return out
}

// wildcardMatch matches a name the way doctest's test-case filters do: the
// whole name, * for any run of characters, ? for one, ignoring case.
func wildcardMatch(pattern, name string) bool {
	p, s := []rune(strings.ToLower(pattern)), []rune(strings.ToLower(name))
	star, mark := -1, 0
	i, j := 0, 0
	for j < len(s) {
		switch {
		case i < len(p) && p[i] == '*':
			star, mark = i, j
			i++
		case i < len(p) && (p[i] == '?' || p[i] == s[j]):
			i++
			j++
		case star >= 0:
			i, mark = star+1, mark+1
			j = mark
		default:
			return false
		}
	}
	for i < len(p) && p[i] == '*' {
		i++
	}
	return i == len(p)
}

func matchesAny(patterns []string, name string) bool {
	return slices.ContainsFunc(patterns, func(p string) bool { return wildcardMatch(p, name) })
}

// validFolderName: a project's folder sits directly under work/.
func validFolderName(id string) bool {
	return id != "" && !strings.HasPrefix(id, ".") && !strings.ContainsAny(id, `/\`) && id != "build"
}

// validateProjectLesson checks what only a project lesson has: its front
// matter, its contract and what it retires.
func (c *Course) validateProjectLesson(l *Lesson) []string {
	p, m := l.project, l.Meta
	var errs []string
	add := func(format string, a ...any) { errs = append(errs, l.ID+": "+fmt.Sprintf(format, a...)) }
	first := p.Lessons[0] == l

	if m.Base != "" {
		add("base and project can't both be set")
	}
	if len(m.Files) > 0 {
		add("a project lesson has no files: list; you name your own files")
	}
	if m.Build != "" || m.Check != "" || l.IsQuiz() {
		add("a project lesson is graded by its tests, so it has no build:, check: or quiz")
	}
	if !validFolderName(p.ID) || c.lessons[p.ID] != nil {
		add("project %q must be a plain folder name that isn't a lesson id", p.ID)
	}
	if first && m.Contract.Header == "" {
		add("the first lesson of a project names the contract header")
	}
	if h := m.Contract.Header; h != "" && h != p.Header {
		add("contract header %s isn't the project's header %s", h, p.Header)
	}
	if !first {
		if starter, _ := l.files("starter"); len(starter) > 0 {
			add("only a project's first lesson can have a starter/")
		}
	}
	if sol, err := l.Solution(); err != nil {
		add("%v", err)
	} else if _, ok := sol[p.Header]; !ok && p.Header != "" {
		add("the reference has no %s", p.Header)
	}

	var before []ContractItem
	if prev := p.prev(l); prev != nil {
		before = prev.Contract()
	}
	for _, d := range m.Contract.Drops {
		if !slices.ContainsFunc(before, func(it ContractItem) bool { return it.Name == d }) {
			add("contract drops %s, which no earlier lesson added", d)
		}
	}
	for _, t := range m.Contract.Types {
		if !cppName.MatchString(t) {
			add("contract type %q isn't a C++ name", t)
		}
	}
	for _, f := range m.Contract.Functions {
		if !cppName.MatchString(f.Name) {
			add("contract function %q isn't a C++ name", f.Name)
		}
		if _, _, _, err := splitSig(f.Sig); err != nil {
			add("contract function %s: %v", f.Name, err)
		}
	}

	for _, r := range m.Retires {
		x := c.lessons[r.Lesson]
		if x == nil || x.project != p || p.index(x) >= p.index(l) {
			add("retires %s, which isn't an earlier lesson of project %s", r.Lesson, p.ID)
			continue
		}
		tests, _ := x.tests()
		for _, f := range r.Tests {
			if _, ok := tests[f]; !ok {
				add("retires %s's tests/%s, which it doesn't have", x.ID, f)
			}
		}
		names := testCaseNames(tests)
		for _, pat := range r.Cases {
			switch {
			case strings.Contains(pat, ","):
				add("retired test-case pattern %q can't contain a comma", pat)
			case !slices.ContainsFunc(names, func(n string) bool { return wildcardMatch(pat, n) }):
				add("retires test cases %q, which match none of %s's", pat, x.ID)
			}
		}
		checks, _ := x.Checks()
		for _, name := range r.Checks {
			if !slices.ContainsFunc(checks, func(ch Check) bool { return ch.Name == name }) {
				add("retires check %q, which %s's checks.yaml doesn't have", name, x.ID)
			}
		}
	}
	return errs
}

// errNotProject is returned by the project-only actions.
var errNotProject = errors.New("only a project lesson has a reference to catch up to")
