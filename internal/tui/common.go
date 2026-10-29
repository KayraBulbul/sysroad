package tui

import (
	"context"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"runtime"
	"strings"
	"time"

	tea "github.com/charmbracelet/bubbletea"
	"gopkg.in/yaml.v3"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/gitsync"
	"github.com/KayraBulbul/sysroad/internal/roadmap"
	"github.com/KayraBulbul/sysroad/internal/store"
)

const (
	pushDelay    = 15 * time.Second // after the last change
	pullInterval = 5 * time.Minute
	retryDelay   = 2 * time.Minute
)

type tickMsg time.Time

func tick() tea.Cmd {
	return tea.Tick(time.Second, func(t time.Time) tea.Msg { return tickMsg(t) })
}

type syncDoneMsg struct{ err error }

func syncCmd(dir string) tea.Cmd {
	return func() tea.Msg {
		ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
		defer cancel()
		return syncDoneMsg{gitsync.Sync(ctx, dir)}
	}
}

type editorDoneMsg struct {
	what, id, path string
	err            error
}

// core is the state shared by the full-screen and sidebar views.
type core struct {
	app  *app.App
	w, h int

	hasRepo   bool
	hasRemote bool
	syncing   bool
	dirty     bool
	lastWrite time.Time
	lastSync  time.Time
	retryAt   time.Time
	syncErr   error

	flash    string
	flashErr bool
	flashAt  time.Time
}

func newCore(a *app.App) core {
	c := core{app: a, hasRepo: gitsync.IsRepo(a.Dir)}
	c.hasRemote = c.hasRepo && gitsync.HasRemote(context.Background(), a.Dir)
	return c
}

// Dirty reports whether there are changes that haven't been synced yet.
func (c *core) Dirty() bool { return c.dirty || c.syncing }

func (c *core) init() tea.Cmd {
	if c.hasRepo {
		c.syncing = true
		return tea.Batch(tick(), syncCmd(c.app.Dir))
	}
	return tick()
}

func (c *core) changed() {
	c.dirty = true
	c.lastWrite = time.Now()
}

func (c *core) say(format string, args ...any) {
	c.flash, c.flashErr, c.flashAt = fmt.Sprintf(format, args...), false, time.Now()
}

func (c *core) fail(err error) {
	if err != nil {
		c.flash, c.flashErr, c.flashAt = err.Error(), true, time.Now()
	}
}

func (c *core) check(err error) {
	if err != nil {
		c.fail(err)
	} else {
		c.changed()
	}
}

func (c *core) syncNow() tea.Cmd {
	if !c.hasRepo || c.syncing {
		return nil
	}
	c.syncing, c.dirty = true, false
	return syncCmd(c.app.Dir)
}

// update handles messages common to both views.
func (c *core) update(msg tea.Msg) (tea.Cmd, bool) {
	switch msg := msg.(type) {
	case tea.WindowSizeMsg:
		c.w, c.h = msg.Width, msg.Height
		return nil, true
	case tickMsg:
		if _, err := c.app.Reload(false); err != nil {
			c.fail(err)
		}
		now := time.Time(msg)
		if c.flash != "" && now.Sub(c.flashAt) > 5*time.Second {
			c.flash = ""
		}
		due := (c.dirty && now.Sub(c.lastWrite) >= pushDelay) || now.Sub(c.lastSync) >= pullInterval
		if c.hasRepo && !c.syncing && now.After(c.retryAt) && due {
			return tea.Batch(tick(), c.syncNow()), true
		}
		return tick(), true
	case syncDoneMsg:
		c.syncing = false
		c.lastSync = time.Now()
		switch {
		case errors.Is(msg.err, gitsync.ErrBusy):
			c.dirty = true
		case msg.err != nil:
			c.syncErr, c.dirty, c.retryAt = msg.err, true, time.Now().Add(retryDelay)
		default:
			c.syncErr = nil
		}
		if _, err := c.app.Reload(false); err != nil {
			c.fail(err)
		}
		return nil, true
	case editorDoneMsg:
		c.applyEdit(msg)
		return nil, true
	}
	return nil, false
}

func (c *core) syncStatus() string {
	switch {
	case !c.hasRepo:
		return sDim.Render("local only")
	case c.syncing:
		return sWarn.Render("⟳ syncing")
	case c.syncErr != nil:
		return sErr.Render("✗ sync failed")
	case c.dirty:
		return sWarn.Render("● unsynced")
	case c.lastSync.IsZero():
		return ""
	case !c.hasRemote:
		return sDim.Render("✓ committed (no remote)")
	default:
		return sGood.Render("✓") + sDim.Render(" synced "+ago(c.lastSync))
	}
}

func ago(t time.Time) string {
	d := time.Since(t)
	switch {
	case d < time.Minute:
		return "now"
	case d < time.Hour:
		return fmt.Sprintf("%dm ago", int(d.Minutes()))
	default:
		return fmt.Sprintf("%dh ago", int(d.Hours()))
	}
}

func clock(d time.Duration) string {
	d = d.Round(time.Second)
	return fmt.Sprintf("%d:%02d:%02d", int(d.Hours()), int(d.Minutes())%60, int(d.Seconds())%60)
}

// streakText is "streak 4d" style header text.
func (c *core) streakText() string {
	cur, best := c.app.Progress.Streak(time.Now())
	s := fmt.Sprintf("streak %dd", cur)
	if best > cur {
		s += sDim.Render(fmt.Sprintf(" (best %d)", best))
	}
	return s
}

func (c *core) timerText() string {
	t := c.app.Timer
	if t == nil {
		return ""
	}
	return sWarn.Render("◷ "+clock(t.Elapsed(time.Now()))) + " " + trunc(c.app.Roadmap.TitleOf(t.ID), 28)
}

func (c *core) toggleTimer(id string) {
	if t := c.app.Timer; t != nil && t.ID == id {
		mins, err := c.app.StopTimer()
		c.check(err)
		if err == nil {
			c.say("Logged %s on %s", store.FormatMinutes(mins), trunc(c.app.Roadmap.TitleOf(id), 40))
		}
		return
	}
	c.check(c.app.StartTimer(id))
}

func (c *core) toggleStatus(id string, s store.Status) {
	if c.app.Progress.StatusOf(id) == s {
		s = store.Todo
	}
	if s == store.Done {
		if t := c.app.Timer; t != nil && t.ID == id {
			_, err := c.app.StopTimer()
			c.fail(err)
		}
	}
	c.check(c.app.SetStatus(id, s))
}

func editorCommand() string {
	for _, v := range []string{"VISUAL", "EDITOR"} {
		if e := os.Getenv(v); e != "" {
			return e
		}
	}
	if _, err := exec.LookPath("nvim"); err == nil {
		return "nvim"
	}
	return "vi"
}

// edit opens content in $EDITOR; the result arrives as editorDoneMsg.
func edit(what, id, content, suffix string) tea.Cmd {
	f, err := os.CreateTemp("", "sysroad-*"+suffix)
	if err != nil {
		return func() tea.Msg { return editorDoneMsg{what: what, id: id, err: err} }
	}
	f.WriteString(content)
	f.Close()
	// Run through sh so EDITOR values with arguments ("code -w") work.
	cmd := exec.Command("sh", "-c", editorCommand()+` "$1"`, "sh", f.Name())
	return tea.ExecProcess(cmd, func(err error) tea.Msg {
		return editorDoneMsg{what: what, id: id, path: f.Name(), err: err}
	})
}

func (c *core) editNote(id string) tea.Cmd {
	return edit("note", id, c.app.Progress.Notes[id], ".md")
}

func (c *core) editStep(id string) tea.Cmd {
	_, st := c.app.Roadmap.Find(id)
	data, err := roadmap.Marshal(st)
	if err != nil {
		c.fail(err)
		return nil
	}
	return edit("step", id, "# Edit this step. kinds: "+strings.Join(roadmap.Kinds, ", ")+"\n"+string(data), ".yaml")
}

// sectionMeta is a section without its steps, for editing.
type sectionMeta struct {
	ID        string             `yaml:"id"`
	Title     string             `yaml:"title"`
	Summary   string             `yaml:"summary,omitempty"`
	Optional  bool               `yaml:"optional,omitempty"`
	Resources []roadmap.Resource `yaml:"resources,omitempty"`
}

func (c *core) editSection(id string) tea.Cmd {
	s := c.app.Roadmap.Section(id)
	data, err := roadmap.Marshal(sectionMeta{s.ID, s.Title, s.Summary, s.Optional, s.Resources})
	if err != nil {
		c.fail(err)
		return nil
	}
	return edit("section", id, "# Edit this section (steps are edited individually).\n"+string(data), ".yaml")
}

func (c *core) applyEdit(msg editorDoneMsg) {
	if msg.path != "" {
		defer os.Remove(msg.path)
	}
	if msg.err != nil {
		c.fail(fmt.Errorf("editor: %w", msg.err))
		return
	}
	data, err := os.ReadFile(msg.path)
	if err != nil {
		c.fail(err)
		return
	}
	switch msg.what {
	case "note":
		c.check(c.app.SetNote(msg.id, strings.TrimSpace(string(data))))
	case "step":
		_, st := c.app.Roadmap.Find(msg.id)
		if st == nil {
			c.fail(errors.New("step no longer exists"))
			return
		}
		var edited roadmap.Step
		if err := yaml.Unmarshal(data, &edited); err != nil {
			c.fail(fmt.Errorf("not saved — bad YAML: %w", err))
			return
		}
		edited.ID = st.ID // IDs are referenced by the progress log
		*st = edited
		c.check(c.app.SaveRoadmap())
	case "section":
		s := c.app.Roadmap.Section(msg.id)
		if s == nil {
			c.fail(errors.New("section no longer exists"))
			return
		}
		var m sectionMeta
		if err := yaml.Unmarshal(data, &m); err != nil {
			c.fail(fmt.Errorf("not saved — bad YAML: %w", err))
			return
		}
		s.Title, s.Summary, s.Optional, s.Resources = m.Title, m.Summary, m.Optional, m.Resources
		c.check(c.app.SaveRoadmap())
	}
}

func openURL(url string) error {
	if url == "" {
		return errors.New("no link for this item")
	}
	name := "xdg-open"
	if runtime.GOOS == "darwin" {
		name = "open"
	}
	cmd := exec.Command(name, url)
	if err := cmd.Start(); err != nil {
		return err
	}
	go cmd.Wait()
	return nil
}

// sectionURL is the first linked resource of a section.
func sectionURL(s *roadmap.Section) string {
	for _, r := range s.Resources {
		if r.URL != "" {
			return r.URL
		}
	}
	return ""
}
