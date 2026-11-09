// Package course is the built-in C++ systems course: its structure
// (phases → chapters → lessons), the content of each written lesson, and the
// engine that builds and tests a learner's code (see runner.go).
//
// Content ships inside the binary (content/). A learner's code lives in the
// data repo under work/<lesson-id>/ so it syncs between machines.
package course

import (
	"embed"
	"errors"
	"fmt"
	"io/fs"
	"path"
	"sort"
	"strings"

	"gopkg.in/yaml.v3"

	"github.com/KayraBulbul/sysroad/internal/roadmap"
)

//go:embed all:content
var content embed.FS

// Lesson kinds. Code lessons are graded by Submit; the rest are ticked off by hand.
const (
	KindLesson   = "lesson"   // short concept + task, graded
	KindProject  = "project"  // bigger graded task, fewer instructions
	KindCapstone = "capstone" // capstone milestone, spec + acceptance tests only
	KindQuiz     = "quiz"     // multiple choice
	KindSetup    = "setup"    // machine setup; may have a built-in check
	KindLab      = "lab"      // external lab (CSAPP, xv6), ticked off by hand
	KindVideo    = "video"    // optional watching
	KindRead     = "read"     // optional reading
)

type Course struct {
	Title  string   `yaml:"title"`
	Phases []*Phase `yaml:"phases"`

	lessons map[string]*Lesson
}

type Phase struct {
	ID       string     `yaml:"id"`
	Title    string     `yaml:"title"`
	Label    string     `yaml:"label,omitempty"` // shown instead of the phase number, e.g. "★"
	Summary  string     `yaml:"summary,omitempty"`
	Optional bool       `yaml:"optional,omitempty"`
	Chapters []*Chapter `yaml:"chapters"`
}

type Chapter struct {
	ID       string    `yaml:"id"`
	Title    string    `yaml:"title"`
	Summary  string    `yaml:"summary,omitempty"`
	Project  string    `yaml:"project,omitempty"` // the running project this chapter builds
	Optional bool      `yaml:"optional,omitempty"`
	Lessons  []*Lesson `yaml:"lessons"`

	Phase  *Phase `yaml:"-"`
	Number string `yaml:"-"` // "1.2"
}

// Lesson is one entry in a chapter. Fields set in course.yaml can be
// overridden by the lesson's own lesson.md front matter.
type Lesson struct {
	ID       string `yaml:"id"`
	Title    string `yaml:"title"`
	Kind     string `yaml:"kind,omitempty"`
	Minutes  int    `yaml:"minutes,omitempty"`
	XP       int    `yaml:"xp,omitempty"`
	Optional bool   `yaml:"optional,omitempty"`
	Platform string `yaml:"platform,omitempty"`
	URL      string `yaml:"url,omitempty"`
	Detail   string `yaml:"detail,omitempty"` // for hand-ticked steps without a lesson folder

	Chapter *Chapter `yaml:"-"`
	Written bool     `yaml:"-"` // has a content/lessons/<id>/ folder
	Meta    Meta     `yaml:"-"`
	Body    string   `yaml:"-"` // lesson.md without front matter
}

// Meta is a written lesson's lesson.md front matter.
type Meta struct {
	Title   string   `yaml:"title"`
	Kind    string   `yaml:"kind"`
	Minutes int      `yaml:"minutes"`
	XP      int      `yaml:"xp"`
	Files   []string `yaml:"files"` // files you edit; the first is opened by `sysroad edit`
	Check   string   `yaml:"check"` // built-in check instead of tests, e.g. "toolchain"
	Video   *Link    `yaml:"video"`
	Deeper  []Link   `yaml:"deeper"` // optional reference links
	Run     *Sample  `yaml:"run"`    // sample run shown by Run
	// Optimize builds the program (not the tests) with -O2 and no
	// sanitizers, for lessons that measure speed.
	Optimize bool `yaml:"optimize"`
}

type Link struct {
	Title string `yaml:"title" json:"title"`
	URL   string `yaml:"url" json:"url"`
}

// Sample is how Run invokes your program to show what it prints.
type Sample struct {
	Args        []string          `yaml:"args"`
	Stdin       string            `yaml:"stdin"`
	Files       map[string]string `yaml:"files"`
	Interactive bool              `yaml:"interactive"` // hand the program the terminal
}

// Check is one program-level test from checks.yaml: run the program with
// these inputs and compare what it prints.
type Check struct {
	Name   string            `yaml:"name"`
	Args   []string          `yaml:"args"`
	Stdin  string            `yaml:"stdin"`
	Files  map[string]string `yaml:"files"`
	Stdout *string           `yaml:"stdout"`
	Exit   *int              `yaml:"exit"`
	Hidden bool              `yaml:"hidden"`
}

// Graded reports whether Submit (rather than a manual tick) completes it.
// Quizzes count: answering every question right completes them.
func (l *Lesson) Graded() bool {
	return l.Written && (l.Meta.Check != "" || len(l.Meta.Files) > 0 || l.IsQuiz())
}

// HasCode reports whether the lesson has code you write in its work folder,
// so editing, Reset and the solution apply (built-in checks have none).
func (l *Lesson) HasCode() bool { return l.Graded() && l.Meta.Check == "" && len(l.Meta.Files) > 0 }

func (l *Lesson) dir() string { return path.Join("content/lessons", l.ID) }

func (l *Lesson) read(name string) (string, bool) {
	data, err := content.ReadFile(path.Join(l.dir(), name))
	if err != nil {
		return "", false
	}
	return string(data), true
}

// Hints are hints.md split on lines containing only "---".
func (l *Lesson) Hints() []string {
	s, ok := l.read("hints.md")
	if !ok {
		return nil
	}
	return splitRule(s)
}

func (l *Lesson) UnderTheHood() string {
	s, _ := l.read("under_the_hood.md")
	return strings.TrimSpace(s)
}

func (l *Lesson) Checks() ([]Check, error) {
	s, ok := l.read("checks.yaml")
	if !ok {
		return nil, nil
	}
	var cs []Check
	if err := yaml.Unmarshal([]byte(s), &cs); err != nil {
		return nil, fmt.Errorf("%s/checks.yaml: %w", l.ID, err)
	}
	return cs, nil
}

// files lists the paths (relative to the lesson's sub dir) of a lesson
// sub-directory such as "starter" or "solution".
func (l *Lesson) files(sub string) (map[string][]byte, error) {
	root := path.Join(l.dir(), sub)
	out := map[string][]byte{}
	err := fs.WalkDir(content, root, func(p string, d fs.DirEntry, err error) error {
		if err != nil {
			if errors.Is(err, fs.ErrNotExist) {
				return fs.SkipAll
			}
			return err
		}
		if d.IsDir() {
			return nil
		}
		data, err := content.ReadFile(p)
		if err != nil {
			return err
		}
		out[strings.TrimPrefix(p, root+"/")] = data
		return nil
	})
	return out, err
}

func (l *Lesson) Starter() (map[string][]byte, error)  { return l.files("starter") }
func (l *Lesson) Solution() (map[string][]byte, error) { return l.files("solution") }
func (l *Lesson) tests() (map[string][]byte, error)    { return l.files("tests") }

// SortedNames returns map keys in order, primary files first.
func (l *Lesson) SortedNames(m map[string][]byte) []string {
	rank := map[string]int{}
	for i, f := range l.Meta.Files {
		rank[f] = i + 1
	}
	names := make([]string, 0, len(m))
	for k := range m {
		names = append(names, k)
	}
	sort.Slice(names, func(i, j int) bool {
		ri, rj := rank[names[i]], rank[names[j]]
		if ri == 0 {
			ri = 1 << 20
		}
		if rj == 0 {
			rj = 1 << 20
		}
		if ri != rj {
			return ri < rj
		}
		return names[i] < names[j]
	})
	return names
}

func splitRule(s string) []string {
	var parts []string
	var cur []string
	flush := func() {
		if t := strings.TrimSpace(strings.Join(cur, "\n")); t != "" {
			parts = append(parts, t)
		}
		cur = nil
	}
	for _, line := range strings.Split(s, "\n") {
		if strings.TrimSpace(line) == "---" {
			flush()
			continue
		}
		cur = append(cur, line)
	}
	flush()
	return parts
}

// Load parses the built-in course.
func Load() (*Course, error) {
	data, err := content.ReadFile("content/course.yaml")
	if err != nil {
		return nil, err
	}
	var c Course
	if err := yaml.Unmarshal(data, &c); err != nil {
		return nil, fmt.Errorf("course.yaml: %w", err)
	}
	c.lessons = map[string]*Lesson{}
	seen := map[string]bool{}
	claim := func(id, what string) error {
		if id == "" {
			return fmt.Errorf("course.yaml: %s has an empty id", what)
		}
		if seen[id] {
			return fmt.Errorf("course.yaml: duplicate id %q", id)
		}
		seen[id] = true
		return nil
	}
	for pi, p := range c.Phases {
		if err := claim(p.ID, "phase "+p.Title); err != nil {
			return nil, err
		}
		label := p.Label
		if label == "" {
			label = fmt.Sprint(pi)
		}
		for ci, ch := range p.Chapters {
			if err := claim(ch.ID, "chapter "+ch.Title); err != nil {
				return nil, err
			}
			ch.Phase = p
			ch.Number = fmt.Sprintf("%s.%d", label, ci+1)
			for _, l := range ch.Lessons {
				if err := claim(l.ID, "lesson "+l.Title); err != nil {
					return nil, err
				}
				l.Chapter = ch
				if err := l.load(); err != nil {
					return nil, err
				}
				c.lessons[l.ID] = l
			}
		}
	}
	return &c, nil
}

func (l *Lesson) load() error {
	md, ok := l.read("lesson.md")
	if !ok {
		if l.Kind == "" {
			l.Kind = KindLesson
		}
		return nil
	}
	l.Written = true
	meta, body, err := frontMatter(md)
	if err != nil {
		return fmt.Errorf("lesson %s: %w", l.ID, err)
	}
	if err := yaml.Unmarshal([]byte(meta), &l.Meta); err != nil {
		return fmt.Errorf("lesson %s front matter: %w", l.ID, err)
	}
	l.Body = strings.TrimSpace(body)
	if l.Meta.Title != "" {
		l.Title = l.Meta.Title
	}
	if l.Meta.Kind != "" {
		l.Kind = l.Meta.Kind
	}
	if l.Meta.Minutes != 0 {
		l.Minutes = l.Meta.Minutes
	}
	if l.Meta.XP != 0 {
		l.XP = l.Meta.XP
	}
	if l.Kind == "" {
		l.Kind = KindLesson
	}
	return nil
}

func frontMatter(md string) (meta, body string, err error) {
	if !strings.HasPrefix(md, "---\n") {
		return "", md, nil
	}
	end := strings.Index(md[4:], "\n---\n")
	if end < 0 {
		return "", "", errors.New("unterminated front matter")
	}
	return md[4 : 4+end], md[4+end+5:], nil
}

// Lesson looks a lesson up by ID.
func (c *Course) Lesson(id string) *Lesson { return c.lessons[id] }

// Chapters returns every chapter in course order.
func (c *Course) Chapters() []*Chapter {
	var out []*Chapter
	for _, p := range c.Phases {
		out = append(out, p.Chapters...)
	}
	return out
}

// Lessons returns every lesson in course order.
func (c *Course) Lessons() []*Lesson {
	var out []*Lesson
	for _, ch := range c.Chapters() {
		out = append(out, ch.Lessons...)
	}
	return out
}

// XPFor is the XP a lesson is worth when completed.
func XPFor(l *Lesson) int {
	if l.XP > 0 {
		return l.XP
	}
	switch l.Kind {
	case KindCapstone:
		return 40
	case KindProject, KindLab:
		return 25
	case KindQuiz, KindSetup, KindVideo, KindRead:
		return 5
	default:
		return 10
	}
}

// Level turns total XP into a level and progress within it. Level n starts
// at 25·(n-1)² XP, so early levels come quickly and later ones take longer.
func Level(xp int) (level, into, span int) {
	level = 1
	for 25*level*level <= xp {
		level++
	}
	start := 25 * (level - 1) * (level - 1)
	return level, xp - start, 25*level*level - start
}

// Roadmap presents the course as read-only roadmap sections (one per
// chapter) so the existing views, stats, streaks and timers work on it.
func (c *Course) Roadmap() *roadmap.Roadmap {
	r := &roadmap.Roadmap{Title: c.Title}
	for _, ch := range c.Chapters() {
		sec := &roadmap.Section{
			ID:       ch.ID,
			Title:    ch.Number + " · " + ch.Title,
			Group:    ch.Phase.Title,
			Optional: ch.Optional || ch.Phase.Optional,
			ReadOnly: true,
		}
		var sum []string
		if ch.Summary != "" {
			sum = append(sum, ch.Summary)
		}
		if ch.Project != "" {
			sum = append(sum, "Project: "+ch.Project)
		}
		sec.Summary = strings.Join(sum, "\n\n")
		for _, l := range ch.Lessons {
			st := &roadmap.Step{
				ID:       l.ID,
				Title:    l.Title,
				Kind:     l.Kind,
				Hours:    float64(max(l.Minutes, 10)) / 60,
				Optional: l.Optional,
				Platform: l.Platform,
				URL:      l.URL,
				Detail:   l.Detail,
			}
			if !l.Written && l.Detail == "" && (l.Kind == KindLesson || l.Kind == KindProject || l.Kind == KindCapstone || l.Kind == KindQuiz) {
				st.Detail = "Not written yet — it arrives in a later sysroad update."
			}
			sec.Steps = append(sec.Steps, st)
		}
		r.Sections = append(r.Sections, sec)
	}
	return r
}

// Validate is used by tests: every written lesson's files must exist.
func (c *Course) Validate() error {
	var errs []string
	for _, l := range c.Lessons() {
		if !l.Written {
			continue
		}
		if l.Kind == KindQuiz {
			errs = append(errs, l.validateQuiz()...)
			continue
		}
		if l.Meta.Check != "" {
			continue
		}
		starter, _ := l.Starter()
		sol, _ := l.Solution()
		for _, f := range l.Meta.Files {
			if _, ok := starter[f]; !ok {
				errs = append(errs, l.ID+": starter/"+f+" missing")
			}
			if _, ok := sol[f]; !ok {
				errs = append(errs, l.ID+": solution/"+f+" missing")
			}
		}
		if _, err := l.Checks(); err != nil {
			errs = append(errs, err.Error())
		}
	}
	if len(errs) > 0 {
		return errors.New(strings.Join(errs, "\n"))
	}
	return nil
}
