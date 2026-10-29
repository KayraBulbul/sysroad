// Package store keeps progress as an append-only event log (events.jsonl).
// State is rebuilt by replaying the log, so two machines appending
// concurrently can be merged by plain line union.
package store

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"sort"
	"strings"
	"time"
)

type Status string

const (
	Todo    Status = "todo"
	Doing   Status = "doing"
	Done    Status = "done"
	Skipped Status = "skipped"
)

const (
	EvStatus = "status"
	EvNote   = "note"
	EvTime   = "time"
)

type Event struct {
	T       time.Time `json:"t"`
	Type    string    `json:"type"`
	ID      string    `json:"id"`
	Value   string    `json:"value,omitempty"`
	Text    string    `json:"text,omitempty"`
	Minutes int       `json:"min,omitempty"`
	Host    string    `json:"host,omitempty"`
}

type Progress struct {
	Status   map[string]Status
	StatusAt map[string]time.Time
	Notes    map[string]string
	Minutes  map[string]int // per item ID
	DayMins  map[string]int // "2006-01-02" -> minutes
	Active   map[string]bool
	Events   []Event // sorted by time
}

func (p *Progress) StatusOf(id string) Status {
	if s, ok := p.Status[id]; ok {
		return s
	}
	return Todo
}

func Day(t time.Time) string { return t.Local().Format("2006-01-02") }

func Replay(events []Event) *Progress {
	sort.SliceStable(events, func(i, j int) bool { return events[i].T.Before(events[j].T) })
	p := &Progress{
		Status:   map[string]Status{},
		StatusAt: map[string]time.Time{},
		Notes:    map[string]string{},
		Minutes:  map[string]int{},
		DayMins:  map[string]int{},
		Active:   map[string]bool{},
		Events:   events,
	}
	doneDay := map[string]string{}
	for _, e := range events {
		switch e.Type {
		case EvStatus:
			p.Status[e.ID] = Status(e.Value)
			p.StatusAt[e.ID] = e.T
			if Status(e.Value) == Done {
				doneDay[e.ID] = Day(e.T)
			}
		case EvNote:
			if e.Text == "" {
				delete(p.Notes, e.ID)
			} else {
				p.Notes[e.ID] = e.Text
			}
		case EvTime:
			p.Minutes[e.ID] += e.Minutes
			p.DayMins[Day(e.T)] += e.Minutes
			p.Active[Day(e.T)] = true
		}
	}
	// A completion only counts towards the streak if it wasn't undone.
	for id, d := range doneDay {
		if p.Status[id] == Done {
			p.Active[d] = true
		}
	}
	return p
}

// Streak returns the current and longest run of consecutive active days.
// Today not being active yet doesn't break the current streak.
func (p *Progress) Streak(now time.Time) (current, longest int) {
	if len(p.Active) == 0 {
		return 0, 0
	}
	day := func(t time.Time) string { return Day(t) }
	d := now
	if !p.Active[day(d)] {
		d = d.AddDate(0, 0, -1)
	}
	for p.Active[day(d)] {
		current++
		d = d.AddDate(0, 0, -1)
	}
	days := make([]string, 0, len(p.Active))
	for k := range p.Active {
		days = append(days, k)
	}
	sort.Strings(days)
	run := 0
	var prev time.Time
	for _, k := range days {
		t, _ := time.ParseInLocation("2006-01-02", k, time.Local)
		if run > 0 && prev.AddDate(0, 0, 1).Format("2006-01-02") == k {
			run++
		} else {
			run = 1
		}
		prev = t
		longest = max(longest, run)
	}
	return current, longest
}

// AvgMinutes is the average minutes per day over the last n days (including today).
func (p *Progress) AvgMinutes(now time.Time, n int) float64 {
	total := 0
	for i := 0; i < n; i++ {
		total += p.DayMins[Day(now.AddDate(0, 0, -i))]
	}
	return float64(total) / float64(n)
}

func ReadEvents(path string) ([]Event, error) {
	f, err := os.Open(path)
	if errors.Is(err, os.ErrNotExist) {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	defer f.Close()
	var events []Event
	sc := bufio.NewScanner(f)
	sc.Buffer(make([]byte, 1<<20), 1<<24)
	for sc.Scan() {
		line := strings.TrimSpace(sc.Text())
		if line == "" {
			continue
		}
		var e Event
		// Skip corrupt lines (e.g. a torn write) rather than refusing to start.
		if json.Unmarshal([]byte(line), &e) == nil && e.Type != "" {
			events = append(events, e)
		}
	}
	return events, sc.Err()
}

// Append writes one event as a single write(2) on an O_APPEND file, so
// concurrent sysroad processes on one machine don't interleave lines.
func Append(path string, e Event) error {
	if e.T.IsZero() {
		e.T = time.Now()
	}
	e.T = e.T.UTC().Truncate(time.Second)
	if e.Host == "" {
		e.Host = hostname()
	}
	data, err := json.Marshal(e)
	if err != nil {
		return err
	}
	f, err := os.OpenFile(path, os.O_WRONLY|os.O_APPEND|os.O_CREATE, 0o644)
	if err != nil {
		return err
	}
	if _, err := f.Write(append(data, '\n')); err != nil {
		f.Close()
		return err
	}
	return f.Close()
}

func hostname() string {
	h, _ := os.Hostname()
	if i := strings.IndexByte(h, '.'); i > 0 {
		h = h[:i]
	}
	return h
}

// Timer is machine-local (not synced): a running stopwatch on one item.
type Timer struct {
	ID    string    `json:"id"`
	Start time.Time `json:"start"`
}

func (t *Timer) Elapsed(now time.Time) time.Duration { return now.Sub(t.Start) }

func ReadTimer(path string) *Timer {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil
	}
	var t Timer
	if json.Unmarshal(data, &t) != nil || t.ID == "" {
		return nil
	}
	return &t
}

func WriteTimer(path string, t *Timer) error {
	if t == nil {
		err := os.Remove(path)
		if errors.Is(err, os.ErrNotExist) {
			return nil
		}
		return err
	}
	data, _ := json.Marshal(t)
	return os.WriteFile(path, data, 0o644)
}

// FormatMinutes renders 95 as "1h35m".
func FormatMinutes(m int) string {
	if m < 60 {
		return fmt.Sprintf("%dm", m)
	}
	if m%60 == 0 {
		return fmt.Sprintf("%dh", m/60)
	}
	return fmt.Sprintf("%dh%02dm", m/60, m%60)
}

// ParseMinutes accepts "45", "45m", "1h30m", "1.5h".
func ParseMinutes(s string) (int, error) {
	s = strings.TrimSpace(strings.ToLower(s))
	if s == "" {
		return 0, errors.New("empty duration")
	}
	if !strings.ContainsAny(s, "hms") {
		s += "m"
	}
	d, err := time.ParseDuration(s)
	if err != nil {
		return 0, fmt.Errorf("can't parse %q (try 45m, 1h30m, 1.5h)", s)
	}
	if d <= 0 {
		return 0, errors.New("duration must be positive")
	}
	return int(d.Round(time.Minute) / time.Minute), nil
}
