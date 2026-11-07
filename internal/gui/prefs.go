package gui

import (
	"encoding/json"
	"os"
	"path/filepath"

	"github.com/KayraBulbul/sysroad/internal/app"
)

// Prefs are this machine's window settings (pane sizes, vim mode), kept in
// the gitignored .local/ so each machine has its own.
type Prefs map[string]any

func (s *Service) prefsPath() string { return filepath.Join(s.app.Dir, app.LocalDir, "gui.json") }

func (s *Service) Prefs() Prefs {
	s.mu.Lock()
	defer s.mu.Unlock()
	p := Prefs{}
	if data, err := os.ReadFile(s.prefsPath()); err == nil {
		json.Unmarshal(data, &p)
	}
	return p
}

func (s *Service) SavePrefs(p Prefs) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	data, err := json.MarshalIndent(p, "", "  ")
	if err != nil {
		return err
	}
	if err := os.MkdirAll(filepath.Dir(s.prefsPath()), 0o755); err != nil {
		return err
	}
	tmp := s.prefsPath() + ".tmp"
	if err := os.WriteFile(tmp, data, 0o644); err != nil {
		return err
	}
	return os.Rename(tmp, s.prefsPath())
}
