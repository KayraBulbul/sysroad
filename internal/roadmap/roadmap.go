// Package roadmap defines the editable learning roadmap: sections made of
// ordered steps. It is stored as YAML in the data repo.
package roadmap

import (
	"bytes"
	"crypto/rand"
	_ "embed"
	"encoding/hex"
	"fmt"
	"os"
	"regexp"
	"strings"

	"gopkg.in/yaml.v3"
)

//go:embed default.yaml
var DefaultYAML []byte

// Step kinds. Free-form strings are allowed; these get icons and colours.
const (
	KindSetup    = "setup"
	KindRead     = "read"
	KindExercise = "exercise"
	KindLab      = "lab"
	KindProject  = "project"
	KindCapstone = "capstone"
	KindTask     = "task"
)

var Kinds = []string{KindSetup, KindRead, KindExercise, KindLab, KindProject, KindCapstone, KindTask}

type Roadmap struct {
	Title    string     `yaml:"title"`
	Sections []*Section `yaml:"sections"`
}

type Section struct {
	ID        string     `yaml:"id"`
	Title     string     `yaml:"title"`
	Summary   string     `yaml:"summary,omitempty"`
	Optional  bool       `yaml:"optional,omitempty"`
	Resources []Resource `yaml:"resources,omitempty"`
	Steps     []*Step    `yaml:"steps"`
}

type Resource struct {
	Title string `yaml:"title"`
	URL   string `yaml:"url,omitempty"`
	Paid  bool   `yaml:"paid,omitempty"`
	Note  string `yaml:"note,omitempty"`
}

type Step struct {
	ID       string  `yaml:"id"`
	Title    string  `yaml:"title"`
	Kind     string  `yaml:"kind,omitempty"`
	Hours    float64 `yaml:"hours,omitempty"`
	Optional bool    `yaml:"optional,omitempty"`
	Platform string  `yaml:"platform,omitempty"`
	URL      string  `yaml:"url,omitempty"`
	Detail   string  `yaml:"detail,omitempty"`
	DoneWhen string  `yaml:"done_when,omitempty"`
}

func Parse(data []byte) (*Roadmap, error) {
	var r Roadmap
	if err := yaml.Unmarshal(data, &r); err != nil {
		return nil, err
	}
	return &r, r.Validate()
}

func Load(path string) (*Roadmap, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	r, err := Parse(data)
	if err != nil {
		return nil, fmt.Errorf("%s: %w", path, err)
	}
	return r, nil
}

// Validate checks that every section and step has a unique, non-empty ID,
// since progress events reference items by ID.
func (r *Roadmap) Validate() error {
	seen := map[string]bool{}
	check := func(id, what string) error {
		if id == "" {
			return fmt.Errorf("%s has an empty id", what)
		}
		if seen[id] {
			return fmt.Errorf("duplicate id %q", id)
		}
		seen[id] = true
		return nil
	}
	for _, s := range r.Sections {
		if err := check(s.ID, "section "+s.Title); err != nil {
			return err
		}
		for _, st := range s.Steps {
			if err := check(st.ID, "step "+st.Title); err != nil {
				return err
			}
		}
	}
	return nil
}

func Marshal(v any) ([]byte, error) {
	var buf bytes.Buffer
	enc := yaml.NewEncoder(&buf)
	enc.SetIndent(2)
	if err := enc.Encode(v); err != nil {
		return nil, err
	}
	return buf.Bytes(), enc.Close()
}

// Save writes atomically so a concurrently running sidebar never reads a
// half-written file.
func (r *Roadmap) Save(path string) error {
	if err := r.Validate(); err != nil {
		return err
	}
	data, err := Marshal(r)
	if err != nil {
		return err
	}
	tmp := path + ".tmp"
	if err := os.WriteFile(tmp, data, 0o644); err != nil {
		return err
	}
	return os.Rename(tmp, path)
}

func (r *Roadmap) Section(id string) *Section {
	for _, s := range r.Sections {
		if s.ID == id {
			return s
		}
	}
	return nil
}

// Find returns the step with the given ID and its section.
func (r *Roadmap) Find(id string) (*Section, *Step) {
	for _, s := range r.Sections {
		for _, st := range s.Steps {
			if st.ID == id {
				return s, st
			}
		}
	}
	return nil, nil
}

// Title returns the display title of a section or step ID.
func (r *Roadmap) TitleOf(id string) string {
	if s := r.Section(id); s != nil {
		return s.Title
	}
	if _, st := r.Find(id); st != nil {
		return st.Title
	}
	return id
}

var nonSlug = regexp.MustCompile(`[^a-z0-9]+`)

// NewID makes a readable, collision-resistant ID for items created in the TUI.
func NewID(prefix, title string) string {
	slug := strings.Trim(nonSlug.ReplaceAllString(strings.ToLower(title), "-"), "-")
	if len(slug) > 24 {
		slug = strings.TrimRight(slug[:24], "-")
	}
	b := make([]byte, 2)
	_, _ = rand.Read(b)
	parts := []string{}
	for _, p := range []string{prefix, slug, hex.EncodeToString(b)} {
		if p != "" {
			parts = append(parts, p)
		}
	}
	return strings.Join(parts, "-")
}

// Move shifts element i of s by delta, returning the new index.
func Move[T any](s []T, i, delta int) int {
	j := i + delta
	if i < 0 || i >= len(s) || j < 0 || j >= len(s) {
		return i
	}
	s[i], s[j] = s[j], s[i]
	return j
}

func Insert[T any](s []T, i int, v T) []T {
	if i < 0 || i > len(s) {
		i = len(s)
	}
	s = append(s, v)
	copy(s[i+1:], s[i:])
	s[i] = v
	return s
}

func Remove[T any](s []T, i int) []T {
	if i < 0 || i >= len(s) {
		return s
	}
	return append(s[:i], s[i+1:]...)
}

// Change describes one item an upgrade would modify.
type Change struct {
	ID, Title string
	Fields    []string
}

// Upgrade copies descriptive fields from a newer built-in roadmap onto r,
// matching sections and steps by ID. It never adds, removes or reorders
// items, so the user's own structure is kept. With apply false it only
// reports what would change.
func (r *Roadmap) Upgrade(from *Roadmap, apply bool) []Change {
	var changes []Change
	note := func(id, title string, fields []string) {
		if len(fields) > 0 {
			changes = append(changes, Change{id, title, fields})
		}
	}
	for _, s := range r.Sections {
		src := from.Section(s.ID)
		if src == nil {
			continue
		}
		var f []string
		set(&f, "title", &s.Title, src.Title, apply)
		set(&f, "summary", &s.Summary, src.Summary, apply)
		if !resourcesEqual(s.Resources, src.Resources) {
			f = append(f, "resources")
			if apply {
				s.Resources = append([]Resource(nil), src.Resources...)
			}
		}
		note(s.ID, s.Title, f)
		for _, st := range s.Steps {
			_, srcSt := from.Find(st.ID)
			if srcSt == nil {
				continue
			}
			var f []string
			set(&f, "title", &st.Title, srcSt.Title, apply)
			set(&f, "kind", &st.Kind, srcSt.Kind, apply)
			set(&f, "platform", &st.Platform, srcSt.Platform, apply)
			set(&f, "url", &st.URL, srcSt.URL, apply)
			set(&f, "detail", &st.Detail, srcSt.Detail, apply)
			set(&f, "done_when", &st.DoneWhen, srcSt.DoneWhen, apply)
			if st.Hours != srcSt.Hours {
				f = append(f, "hours")
				if apply {
					st.Hours = srcSt.Hours
				}
			}
			if st.Optional != srcSt.Optional {
				f = append(f, "optional")
				if apply {
					st.Optional = srcSt.Optional
				}
			}
			note(st.ID, st.Title, f)
		}
	}
	return changes
}

func set(fields *[]string, name string, dst *string, src string, apply bool) {
	if *dst == src {
		return
	}
	*fields = append(*fields, name)
	if apply {
		*dst = src
	}
}

func resourcesEqual(a, b []Resource) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}
