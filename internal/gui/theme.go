package gui

import (
	_ "embed"
	"maps"
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"sync"
	"time"

	"github.com/fsnotify/fsnotify"
	toml "github.com/pelletier/go-toml/v2"
)

// Theme is an Omarchy colors.toml palette: accent, background, foreground,
// red … bright_magenta. Not every theme has every key (orange and brown are
// often missing); the frontend derives those from the rest.
type Theme struct {
	Name   string            `json:"name"`
	Mode   string            `json:"mode"` // "dark" or "light"
	Colors map[string]string `json:"colors"`
}

//go:embed vigil.toml
var vigilTOML []byte

var hexColor = regexp.MustCompile(`^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$`)

// ParseTheme reads a colors.toml. Non-colour values are kept out of the
// palette; mode defaults to dark.
func ParseTheme(name string, data []byte) (Theme, error) {
	var raw map[string]any
	if err := toml.Unmarshal(data, &raw); err != nil {
		return Theme{}, err
	}
	t := Theme{Name: name, Mode: "dark", Colors: map[string]string{}}
	if m, _ := raw["mode"].(string); m == "light" {
		t.Mode = "light"
	}
	for k, v := range raw {
		if s, ok := v.(string); ok && hexColor.MatchString(s) {
			t.Colors[k] = strings.ToLower(s)
		}
	}
	return t, nil
}

// Vigil is the built-in theme: the Mac's always, and the fallback elsewhere.
func Vigil() Theme {
	t, err := ParseTheme("vigil", vigilTOML)
	if err != nil {
		panic(err) // embedded, so a test catches it
	}
	return t
}

// OmarchyThemeDir is where Omarchy keeps the active theme. omarchy-theme-set
// deletes it and moves the next theme into its place, so watch its parent.
func OmarchyThemeDir() string {
	home, _ := os.UserHomeDir()
	return filepath.Join(home, ".local", "state", "omarchy", "current", "theme")
}

// LoadTheme returns the Omarchy theme in dir if there is one, else Vigil.
func LoadTheme(dir string) Theme {
	if t, ok := readTheme(dir); ok {
		return t
	}
	return Vigil()
}

// readTheme reads the theme in dir, failing while it's missing or half
// written (in the middle of a theme switch).
func readTheme(dir string) (Theme, bool) {
	if dir == "" {
		return Theme{}, false
	}
	data, err := os.ReadFile(filepath.Join(dir, "colors.toml"))
	if err != nil {
		return Theme{}, false
	}
	name := "omarchy"
	if b, err := os.ReadFile(filepath.Join(filepath.Dir(dir), "theme.name")); err == nil {
		name = strings.TrimSpace(string(b))
	}
	t, err := ParseTheme(name, data)
	if err != nil || t.Colors["background"] == "" || t.Colors["foreground"] == "" {
		return Theme{}, false
	}
	return t, true
}

// ThemeDir is the Omarchy theme folder to follow, or "" to use Vigil: the
// Mac always uses Vigil.
func ThemeDir() string {
	if runtime.GOOS != "linux" {
		return ""
	}
	return OmarchyThemeDir()
}

// WatchTheme calls onChange with the new theme whenever the theme in dir
// changes, until stop is closed. It watches dir, its parent and grandparent
// and re-adds the watches after every event, so a swapped or recreated
// folder keeps being followed.
func WatchTheme(dir string, onChange func(Theme), stop <-chan struct{}) error {
	w, err := fsnotify.NewWatcher()
	if err != nil {
		return err
	}
	dirs := []string{filepath.Dir(filepath.Dir(dir)), filepath.Dir(dir), dir}
	rewatch := func() {
		for _, d := range dirs {
			w.Add(d) // a missing dir is fine; it's retried on the next event
		}
	}
	rewatch()
	last := LoadTheme(dir) // before returning, so no change is missed
	go func() {
		defer w.Close()
		var mu sync.Mutex
		var timer *time.Timer
		fire := func() {
			mu.Lock()
			defer mu.Unlock()
			rewatch()
			t, ok := readTheme(dir)
			if ok && !sameTheme(t, last) {
				last = t
				onChange(t)
			}
		}
		for {
			select {
			case <-stop:
				if timer != nil {
					timer.Stop()
				}
				return
			case _, ok := <-w.Events:
				if !ok {
					return
				}
				// The swap is several file operations; settle first.
				if timer != nil {
					timer.Stop()
				}
				timer = time.AfterFunc(150*time.Millisecond, fire)
			case <-w.Errors:
			}
		}
	}()
	return nil
}

func sameTheme(a, b Theme) bool {
	return a.Name == b.Name && a.Mode == b.Mode && maps.Equal(a.Colors, b.Colors)
}
