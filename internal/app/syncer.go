package app

import (
	"context"
	"errors"
	"time"

	"github.com/KayraBulbul/sysroad/internal/gitsync"
)

// Sync timing shared by every long-running front end (TUI and GUI).
const (
	PushDelay    = 15 * time.Second // after the last change
	PullInterval = 5 * time.Minute
	RetryDelay   = 2 * time.Minute
)

// Syncer decides when a long-running front end should sync the data repo:
// shortly after a change, every few minutes to pull, and after a back-off when
// a sync fails. It holds no lock: the caller serialises calls (the TUI's
// update loop, the GUI's mutex) and runs Sync off its main thread.
type Syncer struct {
	Dir       string
	HasRepo   bool
	HasRemote bool

	Syncing   bool
	Dirty     bool
	LastWrite time.Time
	LastSync  time.Time
	RetryAt   time.Time
	Err       error

	lastRun time.Time // newest Run/Submit result seen
}

// NewSyncer probes the data repo. It runs git, so call it once at startup.
func NewSyncer(a *App) *Syncer {
	s := &Syncer{Dir: a.Dir, HasRepo: gitsync.IsRepo(a.Dir)}
	s.HasRemote = s.HasRepo && gitsync.HasRemote(context.Background(), a.Dir)
	_, s.lastRun = a.Engine.Latest()
	return s
}

// Changed notes a local change that should be pushed soon.
func (s *Syncer) Changed(now time.Time) {
	s.Dirty = true
	s.LastWrite = now
}

// Unsynced reports whether there are changes that haven't been synced yet.
func (s *Syncer) Unsynced() bool { return s.Dirty || s.Syncing }

// Begin marks a sync as started, reporting false when there's nothing to
// sync with or one is already running.
func (s *Syncer) Begin() bool {
	if !s.HasRepo || s.Syncing {
		return false
	}
	s.Syncing, s.Dirty = true, false
	return true
}

// Tick is called about once a second with the newest run's time. A run from
// another process (the editor's shell, the CLI) means code changed, so it
// counts as a change. It reports whether a sync should start now, and if so
// has already called Begin.
func (s *Syncer) Tick(now, latestRun time.Time) bool {
	if latestRun.After(s.lastRun) {
		s.lastRun = latestRun
		s.Changed(now)
	}
	due := (s.Dirty && now.Sub(s.LastWrite) >= PushDelay) || now.Sub(s.LastSync) >= PullInterval
	if s.HasRepo && !s.Syncing && now.After(s.RetryAt) && due {
		return s.Begin()
	}
	return false
}

// Run syncs the repo. It touches no Syncer state, so it's safe to call from
// a background goroutine; hand its error to Finish.
func (s *Syncer) Run() error {
	ctx, cancel := context.WithTimeout(context.Background(), time.Minute)
	defer cancel()
	return gitsync.Sync(ctx, s.Dir)
}

// Finish records the outcome of a sync started with Begin or Tick.
func (s *Syncer) Finish(err error, now time.Time) {
	s.Syncing = false
	s.LastSync = now
	switch {
	case errors.Is(err, gitsync.ErrBusy):
		s.Dirty = true
	case err != nil:
		s.Err, s.Dirty, s.RetryAt = err, true, now.Add(RetryDelay)
	default:
		s.Err = nil
	}
}

// SyncState is a front-end-neutral summary of the sync status.
type SyncState string

const (
	SyncLocal     SyncState = "local"     // not a git repo
	SyncBusy      SyncState = "syncing"   // a sync is running
	SyncFailed    SyncState = "failed"    // the last sync failed
	SyncPending   SyncState = "unsynced"  // changes waiting to be pushed
	SyncNever     SyncState = "idle"      // nothing synced yet this session
	SyncCommitted SyncState = "committed" // committed, but there's no remote
	SyncOK        SyncState = "synced"
)

func (s *Syncer) State() SyncState {
	switch {
	case !s.HasRepo:
		return SyncLocal
	case s.Syncing:
		return SyncBusy
	case s.Err != nil:
		return SyncFailed
	case s.Dirty:
		return SyncPending
	case s.LastSync.IsZero():
		return SyncNever
	case !s.HasRemote:
		return SyncCommitted
	default:
		return SyncOK
	}
}
