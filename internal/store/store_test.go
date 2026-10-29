package store

import (
	"path/filepath"
	"testing"
	"time"
)

func day(s string, hour int) time.Time {
	t, _ := time.ParseInLocation("2006-01-02", s, time.Local)
	return t.Add(time.Duration(hour) * time.Hour)
}

func TestReplayLastWriteWinsRegardlessOfLineOrder(t *testing.T) {
	// Union merges can interleave lines from two machines out of time order.
	p := Replay([]Event{
		{T: day("2026-01-02", 10), Type: EvStatus, ID: "a", Value: "done"},
		{T: day("2026-01-01", 10), Type: EvStatus, ID: "a", Value: "doing"},
		{T: day("2026-01-01", 11), Type: EvNote, ID: "a", Text: "first"},
		{T: day("2026-01-03", 11), Type: EvNote, ID: "a", Text: "second"},
		{T: day("2026-01-01", 12), Type: EvTime, ID: "a", Minutes: 30},
		{T: day("2026-01-02", 12), Type: EvTime, ID: "a", Minutes: 45},
	})
	if got := p.StatusOf("a"); got != Done {
		t.Errorf("status = %q, want done", got)
	}
	if got := p.Notes["a"]; got != "second" {
		t.Errorf("note = %q, want second", got)
	}
	if got := p.Minutes["a"]; got != 75 {
		t.Errorf("minutes = %d, want 75", got)
	}
	if got := p.StatusOf("missing"); got != Todo {
		t.Errorf("missing status = %q, want todo", got)
	}
}

func TestEmptyNoteClears(t *testing.T) {
	p := Replay([]Event{
		{T: day("2026-01-01", 1), Type: EvNote, ID: "a", Text: "x"},
		{T: day("2026-01-01", 2), Type: EvNote, ID: "a", Text: ""},
	})
	if _, ok := p.Notes["a"]; ok {
		t.Error("note should be cleared")
	}
}

func TestStreak(t *testing.T) {
	ev := []Event{}
	for _, d := range []string{"2026-01-01", "2026-01-02", "2026-01-03", "2026-01-04", "2026-01-08", "2026-01-09"} {
		ev = append(ev, Event{T: day(d, 12), Type: EvTime, ID: "a", Minutes: 10})
	}
	p := Replay(ev)
	cases := []struct {
		now          string
		cur, longest int
	}{
		{"2026-01-09", 2, 4},
		{"2026-01-10", 2, 4}, // today not active yet: streak still alive
		{"2026-01-11", 0, 4}, // missed yesterday
	}
	for _, c := range cases {
		cur, longest := p.Streak(day(c.now, 20))
		if cur != c.cur || longest != c.longest {
			t.Errorf("Streak(%s) = %d,%d want %d,%d", c.now, cur, longest, c.cur, c.longest)
		}
	}
}

func TestParseMinutes(t *testing.T) {
	ok := map[string]int{"45": 45, "45m": 45, "1h30m": 90, "1.5h": 90, " 2H ": 120, "90s": 2}
	for in, want := range ok {
		if got, err := ParseMinutes(in); err != nil || got != want {
			t.Errorf("ParseMinutes(%q) = %d, %v; want %d", in, got, err, want)
		}
	}
	for _, in := range []string{"", "abc", "-5m", "0"} {
		if _, err := ParseMinutes(in); err == nil {
			t.Errorf("ParseMinutes(%q) should fail", in)
		}
	}
}

func TestAppendAndReadSkipsCorruptLines(t *testing.T) {
	path := filepath.Join(t.TempDir(), "events.jsonl")
	if err := Append(path, Event{Type: EvTime, ID: "a", Minutes: 5}); err != nil {
		t.Fatal(err)
	}
	if err := appendRaw(path, "{\"t\":\"2026-01-01T00:00:00Z\",\"ty"); err != nil {
		t.Fatal(err)
	}
	if err := Append(path, Event{Type: EvTime, ID: "a", Minutes: 7}); err != nil {
		t.Fatal(err)
	}
	ev, err := ReadEvents(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(ev) != 2 || ev[0].Host == "" {
		t.Fatalf("got %d events (%+v), want 2 with host set", len(ev), ev)
	}
}

func TestFormatMinutes(t *testing.T) {
	for in, want := range map[int]string{0: "0m", 59: "59m", 60: "1h", 95: "1h35m"} {
		if got := FormatMinutes(in); got != want {
			t.Errorf("FormatMinutes(%d) = %q want %q", in, got, want)
		}
	}
}

func TestUndoneCompletionDoesNotCountForStreak(t *testing.T) {
	p := Replay([]Event{
		{T: day("2026-01-05", 10), Type: EvStatus, ID: "a", Value: "done"},
		{T: day("2026-01-05", 11), Type: EvStatus, ID: "a", Value: "todo"},
	})
	if cur, _ := p.Streak(day("2026-01-05", 20)); cur != 0 {
		t.Errorf("streak = %d, want 0", cur)
	}
}
