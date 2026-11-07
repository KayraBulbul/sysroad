package app

import (
	"errors"
	"testing"
	"time"
)

// The schedule every front end relies on: push a change after PushDelay, back
// off after a failure, and treat a newer run from another process as a change.
func TestSyncerSchedule(t *testing.T) {
	t0 := time.Date(2026, 11, 7, 12, 0, 0, 0, time.UTC)
	s := &Syncer{HasRepo: true, HasRemote: true, LastSync: t0}

	s.Changed(t0)
	if s.Tick(t0.Add(PushDelay-time.Second), time.Time{}) {
		t.Fatal("synced before the push delay")
	}
	if !s.Tick(t0.Add(PushDelay), time.Time{}) {
		t.Fatal("didn't sync after the push delay")
	}
	if s.Tick(t0.Add(PushDelay+time.Second), time.Time{}) {
		t.Fatal("started a second sync while one was running")
	}

	failAt := t0.Add(PushDelay + 2*time.Second)
	s.Finish(errors.New("offline"), failAt)
	if s.State() != SyncFailed {
		t.Fatalf("state %q after a failure", s.State())
	}
	if s.Tick(failAt.Add(RetryDelay-time.Second), time.Time{}) {
		t.Fatal("retried before the back-off")
	}
	if !s.Tick(failAt.Add(RetryDelay+time.Second), time.Time{}) {
		t.Fatal("didn't retry after the back-off")
	}
	okAt := failAt.Add(RetryDelay + 2*time.Second)
	s.Finish(nil, okAt)
	if s.State() != SyncOK {
		t.Fatalf("state %q after a good sync", s.State())
	}

	runAt := okAt.Add(time.Second)
	if s.Tick(runAt, runAt) || s.State() != SyncPending {
		t.Fatalf("a new run didn't count as a pending change (state %q)", s.State())
	}
	if !s.Tick(runAt.Add(PushDelay), runAt) {
		t.Fatal("didn't push a run's changes")
	}
}
