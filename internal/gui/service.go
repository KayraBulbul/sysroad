// Package gui is the desktop app's Go side: the methods the frontend calls
// and the events it listens to. It doesn't import Wails, so it builds and
// tests anywhere; cmd/sysroad-gui wires it into a window.
package gui

import (
	"context"
	"errors"
	"fmt"
	"strconv"
	"sync"
	"time"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// Events the frontend listens to.
const (
	EvState  = "state"  // progress, sync or run state changed: refetch Snapshot
	EvTheme  = "theme"  // Theme
	EvFile   = "file"   // FileEvent: a lesson file changed on disk
	EvResult = "result" // ResultEvent: a lesson has a new Run/Submit result
)

// Service is bound to the frontend. Wails calls each method on its own
// goroutine, so everything that touches the App holds mu; long work (runs,
// syncs, the first pull of a workspace) happens with mu released.
type Service struct {
	mu   sync.Mutex
	app  *app.App
	sync *app.Syncer

	emit    func(name string, data any)
	openURL func(string) error

	running   *Running
	cancelRun context.CancelFunc

	hintsMod   int64
	latestID   string
	latestAt   time.Time
	themeDir   string
	theme      Theme
	files      *fileWatcher
	stop       chan struct{}
	stopOnce   sync.Once
	syncStatus string // last sync error, for the UI
}

// Running is the Run/Submit in flight.
type Running struct {
	Lesson string      `json:"lesson"`
	Mode   course.Mode `json:"mode"`
	Since  time.Time   `json:"since"`
}

// New opens the service on an App. emit sends an event to the frontend;
// openURL opens a link in the browser.
func New(a *app.App, emit func(string, any), openURL func(string) error) *Service {
	s := &Service{
		app:      a,
		sync:     app.NewSyncer(a),
		emit:     emit,
		openURL:  openURL,
		hintsMod: a.HintsModTime(),
		themeDir: ThemeDir(),
		stop:     make(chan struct{}),
	}
	s.latestID, s.latestAt = a.Engine.Latest()
	s.theme = LoadTheme(s.themeDir)
	s.files = newFileWatcher(a.Engine, func(ev FileEvent) { s.emit(EvFile, ev) })
	return s
}

// Start begins the background work: the first sync, the once-a-second
// reload/sync tick and the theme watcher. Start and Stop are functions, not
// methods, so the frontend bindings don't include them.
func Start(s *Service) {
	if s.themeDir != "" {
		WatchTheme(s.themeDir, func(t Theme) {
			s.mu.Lock()
			s.theme = t
			s.mu.Unlock()
			s.emit(EvTheme, t)
		}, s.stop)
	}
	s.mu.Lock()
	start := s.sync.Begin()
	s.mu.Unlock()
	if start {
		go s.runSync()
	}
	go s.loop()
}

// Stop ends the background work. A sync in flight is left to finish.
func Stop(s *Service) {
	s.stopOnce.Do(func() {
		close(s.stop)
		s.files.close()
		s.mu.Lock()
		if s.cancelRun != nil {
			s.cancelRun()
		}
		s.mu.Unlock()
	})
}

func (s *Service) loop() {
	t := time.NewTicker(time.Second)
	defer t.Stop()
	for {
		select {
		case <-s.stop:
			return
		case now := <-t.C:
			s.tick(now)
		}
	}
}

// tick notices other processes' changes (the TUI, the CLI, a git pull) and
// starts a sync when one is due.
func (s *Service) tick(now time.Time) {
	s.mu.Lock()
	changed, err := s.app.Reload(false)
	if err != nil {
		s.syncStatus = err.Error()
	}
	if m := s.app.HintsModTime(); m != s.hintsMod {
		s.hintsMod, changed = m, true
	}
	var newResult string
	id, at := s.app.Engine.Latest()
	if at.After(s.latestAt) {
		s.latestID, s.latestAt, newResult = id, at, id
		changed = true
	}
	startSync := s.sync.Tick(now, at)
	s.mu.Unlock()

	if newResult != "" {
		s.emit(EvResult, ResultEvent{Lesson: newResult})
	}
	if changed || startSync {
		s.emit(EvState, nil)
	}
	if startSync {
		go s.runSync()
	}
}

func (s *Service) runSync() {
	err := s.sync.Run()
	s.mu.Lock()
	s.sync.Finish(err, time.Now())
	s.app.Reload(false)
	s.mu.Unlock()
	s.emit(EvState, nil)
}

// changed notes a local change to push soon. Hold mu.
func (s *Service) changed() { s.sync.Changed(time.Now()) }

// ---- snapshot ----

// Snapshot is everything the dashboard, phase pages and top bar show.
type Snapshot struct {
	Title    string     `json:"title"`
	Phases   []PhaseDTO `json:"phases"`
	Current  string     `json:"current"` // lesson ID, or ""
	XP       int        `json:"xp"`
	Level    int        `json:"level"`
	Into     int        `json:"into"`
	Span     int        `json:"span"`
	Streak   int        `json:"streak"`
	Best     int        `json:"best"`
	ETA      string     `json:"eta"`
	Today    int        `json:"today"` // minutes
	Done     int        `json:"done"`
	Total    int        `json:"total"`
	Sync     SyncDTO    `json:"sync"`
	Running  *Running   `json:"running"`
	ThemeDir string     `json:"themeDir"`
}

type SyncDTO struct {
	State app.SyncState `json:"state"`
	Error string        `json:"error"`
	At    time.Time     `json:"at"`
}

type PhaseDTO struct {
	ID       string       `json:"id"`
	Number   string       `json:"number"`
	Title    string       `json:"title"`
	Summary  string       `json:"summary"`
	Optional bool         `json:"optional"`
	Done     int          `json:"done"`
	Total    int          `json:"total"`
	Lessons  int          `json:"lessons"` // in the course, written or not
	Chapters []ChapterDTO `json:"chapters"`
}

type ChapterDTO struct {
	ID       string      `json:"id"`
	Number   string      `json:"number"`
	Title    string      `json:"title"`
	Summary  string      `json:"summary"`
	Project  string      `json:"project"`
	Optional bool        `json:"optional"`
	Done     int         `json:"done"`
	Total    int         `json:"total"`
	Lessons  []LessonRow `json:"lessons"`
}

type LessonRow struct {
	ID       string       `json:"id"`
	Title    string       `json:"title"`
	Kind     string       `json:"kind"`
	Minutes  int          `json:"minutes"`
	XP       int          `json:"xp"`
	Status   store.Status `json:"status"`
	Written  bool         `json:"written"`
	Graded   bool         `json:"graded"`
	Optional bool         `json:"optional"`
}

func (s *Service) Snapshot() Snapshot {
	s.mu.Lock()
	defer s.mu.Unlock()
	a := s.app
	now := time.Now()
	snap := Snapshot{Title: a.Course.Title, ThemeDir: s.themeDir}
	snap.XP = a.XP()
	snap.Level, snap.Into, snap.Span = course.Level(snap.XP)
	snap.Streak, snap.Best = a.Progress.Streak(now)
	snap.ETA = a.ETA(now)
	snap.Today = a.TodayMinutes(now)
	if cur, ok := a.Current(); ok && a.Lesson(cur.Step.ID) != nil {
		snap.Current = cur.Step.ID
	}
	snap.Sync = SyncDTO{State: s.sync.State(), At: s.sync.LastSync}
	if s.sync.Err != nil {
		snap.Sync.Error = s.sync.Err.Error()
	} else if s.syncStatus != "" {
		snap.Sync.Error = s.syncStatus
	}
	snap.Running = s.running
	for i, p := range a.Course.Phases {
		pd := PhaseDTO{ID: p.ID, Number: p.Label, Title: p.Title, Summary: p.Summary, Optional: p.Optional}
		if pd.Number == "" {
			pd.Number = strconv.Itoa(i)
		}
		for _, ch := range p.Chapters {
			cd := ChapterDTO{ID: ch.ID, Number: ch.Number, Title: ch.Title, Summary: ch.Summary,
				Project: ch.Project, Optional: ch.Optional || p.Optional}
			if sec := a.Roadmap.Section(ch.ID); sec != nil {
				st := a.SectionStats(sec)
				cd.Done, cd.Total = st.Done, st.Total
			}
			for _, l := range ch.Lessons {
				cd.Lessons = append(cd.Lessons, LessonRow{
					ID: l.ID, Title: l.Title, Kind: l.Kind, Minutes: l.Minutes, XP: course.XPFor(l),
					Status: a.Progress.StatusOf(l.ID), Written: l.Written, Graded: l.Graded(),
					Optional: l.Optional,
				})
			}
			pd.Done += cd.Done
			pd.Total += cd.Total
			pd.Lessons += len(ch.Lessons)
			pd.Chapters = append(pd.Chapters, cd)
		}
		if !p.Optional {
			snap.Done += pd.Done
			snap.Total += pd.Total
		}
		snap.Phases = append(snap.Phases, pd)
	}
	return snap
}

// ---- lesson ----

// LessonDetail is one lesson's page.
type LessonDetail struct {
	LessonRow
	Platform   string         `json:"platform"`
	PhaseID    string         `json:"phaseId"`
	ChapterID  string         `json:"chapterId"`
	Chapter    string         `json:"chapter"` // "1.2 Coreutils I"
	Body       string         `json:"body"`    // markdown
	HasCode    bool           `json:"hasCode"`
	URL        string         `json:"url"`
	Hints      []string       `json:"hints"` // revealed so far
	HintsTotal int            `json:"hintsTotal"`
	HasUTH     bool           `json:"hasUth"`
	UTH        string         `json:"uth"` // only once the lesson is done
	Video      *course.Link   `json:"video"`
	Deeper     []course.Link  `json:"deeper"`
	Note       string         `json:"note"`
	Files      []string       `json:"files"`
	WorkDir    string         `json:"workDir"`
	Result     *course.Result `json:"result"`
	Prev       string         `json:"prev"`
	Next       string         `json:"next"`
}

var errNoLesson = errors.New("no such lesson")

func (s *Service) lesson(id string) (*course.Lesson, error) {
	if l := s.app.Lesson(id); l != nil {
		return l, nil
	}
	return nil, fmt.Errorf("%w: %q", errNoLesson, id)
}

func (s *Service) Lesson(id string) (*LessonDetail, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	a := s.app
	status := a.Progress.StatusOf(l.ID)
	d := &LessonDetail{
		LessonRow: LessonRow{ID: l.ID, Title: l.Title, Kind: l.Kind, Minutes: l.Minutes, XP: course.XPFor(l),
			Status: status, Written: l.Written, Graded: l.Graded(), Optional: l.Optional},
		Platform: l.Platform, HasCode: l.HasCode(), URL: l.URL,
		Video: l.Meta.Video, Deeper: l.Meta.Deeper, Note: a.Progress.Notes[l.ID],
		Files: l.Meta.Files, Result: a.Engine.LastResult(l.ID),
	}
	if ch := l.Chapter; ch != nil {
		d.ChapterID, d.Chapter, d.PhaseID = ch.ID, ch.Number+" "+ch.Title, ch.Phase.ID
		d.Optional = d.Optional || ch.Optional || ch.Phase.Optional
	}
	switch {
	case l.Written:
		d.Body = l.Body
	case l.Detail != "":
		d.Body = l.Detail
	}
	hints := l.Hints()
	d.HintsTotal = len(hints)
	d.Hints = hints[:min(a.HintsShown(l.ID), len(hints))]
	if uth := l.UnderTheHood(); uth != "" {
		d.HasUTH = true
		if status == store.Done {
			d.UTH = uth
		}
	}
	if d.HasCode {
		d.WorkDir = a.Engine.WorkDir(l)
	}
	ls := a.Course.Lessons()
	for i, x := range ls {
		if x.ID == l.ID {
			if i > 0 {
				d.Prev = ls[i-1].ID
			}
			if i+1 < len(ls) {
				d.Next = ls[i+1].ID
			}
		}
	}
	return d, nil
}

// RevealHint shows the next hint and returns the revealed hints.
func (s *Service) RevealHint(id string) ([]string, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.lesson(id)
	if err != nil {
		return nil, err
	}
	n, err := s.app.RevealHint(l)
	if err != nil {
		return nil, err
	}
	s.hintsMod = s.app.HintsModTime()
	return l.Hints()[:n], nil
}

// ToggleDone ticks a hand-checked step off, or back to todo.
func (s *Service) ToggleDone(id string) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	l, err := s.lesson(id)
	if err != nil {
		return err
	}
	if l.Graded() {
		return errors.New("Submit completes this lesson")
	}
	if err := s.app.ToggleStatus(id, store.Done); err != nil {
		return err
	}
	s.changed()
	go s.emit(EvState, nil)
	return nil
}

// ---- runs ----

// RunOutcome is a finished Run/Submit.
type RunOutcome struct {
	Result   *course.Result `json:"result"`
	Outcome  app.Outcome    `json:"outcome"`
	Terminal bool           `json:"terminal"` // the program opened in a terminal window
}

var errBusy = errors.New("a run is already going — one at a time")

// Run builds and tests a lesson. Save the editor first: it runs what's on disk.
func (s *Service) Run(id string, mode course.Mode) (*RunOutcome, error) {
	if mode != course.ModeRun && mode != course.ModeSubmit {
		return nil, fmt.Errorf("unknown mode %q", mode)
	}
	s.mu.Lock()
	l, err := s.lesson(id)
	if err == nil && s.running != nil {
		err = errBusy
	}
	if err == nil {
		err = s.app.BeginRun(l)
	}
	if err != nil {
		s.mu.Unlock()
		return nil, err
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Minute)
	s.running, s.cancelRun = &Running{Lesson: id, Mode: mode, Since: time.Now()}, cancel
	s.changed()
	dir, engine := s.app.Dir, s.app.Engine
	s.mu.Unlock()
	s.emit(EvState, nil)

	res, err := app.Execute(ctx, dir, engine, l, mode)
	ended := ctx.Err() // before cancel(), which would make every run look stopped
	cancel()

	s.mu.Lock()
	s.running, s.cancelRun = nil, nil
	var out RunOutcome
	if err == nil {
		out.Result = res
		out.Outcome, err = s.app.Record(l, res)
		// Our own result: the tick shouldn't announce it as someone else's.
		s.latestID, s.latestAt = engine.Latest()
	}
	s.mu.Unlock()
	s.emit(EvState, nil)
	if err != nil {
		switch {
		case errors.Is(ended, context.Canceled):
			return nil, errors.New("stopped")
		case errors.Is(ended, context.DeadlineExceeded):
			return nil, errors.New("the run took over 3 minutes and was stopped")
		}
		return nil, err
	}
	if res.Interactive != nil {
		if terr := openInTerminal(res.Interactive, l.Title); terr != nil {
			return &out, fmt.Errorf("couldn't open a terminal for the program: %w", terr)
		}
		out.Terminal = true
	}
	return &out, nil
}

// StopRun cancels the Run/Submit in flight.
func (s *Service) StopRun() {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.cancelRun != nil {
		s.cancelRun()
	}
}

// LastResult is a lesson's newest saved result, from any process.
func (s *Service) LastResult(id string) *course.Result {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.app.Engine.LastResult(id)
}

// ---- misc ----

func (s *Service) Theme() Theme {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.theme
}

// SyncNow starts a sync unless one is running.
func (s *Service) SyncNow() {
	s.mu.Lock()
	start := s.sync.Begin()
	s.mu.Unlock()
	if start {
		s.emit(EvState, nil)
		go s.runSync()
	}
}

func (s *Service) OpenURL(url string) error {
	if url == "" {
		return errors.New("no link")
	}
	return s.openURL(url)
}
