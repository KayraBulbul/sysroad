package app

import (
	"errors"
	"testing"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

func firstQuiz(t *testing.T, a *App) *course.Lesson {
	t.Helper()
	for _, l := range a.Course.Lessons() {
		if l.IsQuiz() {
			return l
		}
	}
	t.Skip("no written quiz yet")
	return nil
}

// Answering every question right completes the quiz once, with XP; wrong
// picks are remembered but cost nothing.
func TestQuizCompletesWhenEveryAnswerIsRight(t *testing.T) {
	a := open(t)
	l := firstQuiz(t, a)
	qs, err := l.Quiz()
	if err != nil {
		t.Fatal(err)
	}
	if !errors.Is(Runnable(l), ErrQuiz) {
		t.Fatal("a quiz shouldn't be runnable")
	}

	wrong := (qs[0].Answer + 1) % len(qs[0].Options)
	res, err := a.AnswerQuiz(l, 0, wrong)
	if err != nil {
		t.Fatal(err)
	}
	if res.Correct || res.Quiz.Items[0].Solved || res.Quiz.Items[0].Answer != -1 || res.Quiz.Items[0].Why != "" {
		t.Fatalf("a wrong pick must not reveal the answer: %+v", res.Quiz.Items[0])
	}
	if a.Progress.StatusOf(l.ID) != store.Doing {
		t.Fatal("answering should start the lesson")
	}

	before := a.XP()
	for i, q := range qs {
		res, err = a.AnswerQuiz(l, i, q.Answer)
		if err != nil {
			t.Fatal(err)
		}
		if !res.Correct || !res.Quiz.Items[i].Solved || res.Quiz.Items[i].Why == "" {
			t.Fatalf("question %d: %+v", i+1, res.Quiz.Items[i])
		}
		if last := i == len(qs)-1; res.Outcome.Completed != last {
			t.Fatalf("question %d: completed=%v", i+1, res.Outcome.Completed)
		}
	}
	if a.Progress.StatusOf(l.ID) != store.Done || res.Outcome.XPGained != course.XPFor(l) || a.XP() != before+course.XPFor(l) {
		t.Fatalf("status %s, gained %d", a.Progress.StatusOf(l.ID), res.Outcome.XPGained)
	}
	if got := res.Quiz.Items[0].Tried; len(got) != 2 || got[0] != wrong {
		t.Fatalf("tried = %v", got)
	}

	// Retaking clears the picks but the lesson stays done; finishing again
	// gives no more XP.
	if err := a.ResetQuiz(l); err != nil {
		t.Fatal(err)
	}
	a.Progress.Status[l.ID] = store.Doing // as if it was finished on the other machine…
	q, _ := a.QuizState(l)
	if q.Solved != 0 {
		t.Fatalf("reset left %d solved", q.Solved)
	}
	a.Progress.Status[l.ID] = store.Done // …and done here: every answer shows
	if q, _ = a.QuizState(l); q.Solved != q.Total {
		t.Fatalf("a done quiz should show every answer, got %d/%d", q.Solved, q.Total)
	}
}

func TestQuizRejectsBadPicks(t *testing.T) {
	a := open(t)
	l := firstQuiz(t, a)
	qs, _ := l.Quiz()
	if _, err := a.AnswerQuiz(l, len(qs), 0); err == nil {
		t.Fatal("question out of range accepted")
	}
	if _, err := a.AnswerQuiz(l, 0, len(qs[0].Options)); err == nil {
		t.Fatal("option out of range accepted")
	}
	if _, err := a.QuizState(a.Lesson("cpp-01-echo")); err == nil {
		t.Fatal("cpp-01-echo isn't a quiz")
	}
}
