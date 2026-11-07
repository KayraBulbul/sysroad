package app

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"slices"

	"github.com/KayraBulbul/sysroad/internal/course"
	"github.com/KayraBulbul/sysroad/internal/store"
)

// QuizItem is one question as a front end shows it. The answer and its
// explanation are only filled in once the question is solved.
type QuizItem struct {
	Q       string   `json:"q"`
	Code    string   `json:"code"`
	Options []string `json:"options"`
	Tried   []int    `json:"tried"`  // options picked so far, in order
	Solved  bool     `json:"solved"` // the right option has been picked
	Answer  int      `json:"answer"` // -1 until solved
	Why     string   `json:"why"`    // "" until solved
}

// Quiz is a quiz lesson's questions and how far you've got.
type Quiz struct {
	Items  []QuizItem `json:"items"`
	Solved int        `json:"solved"`
	Total  int        `json:"total"`
}

// Next is the index of the first unsolved question, or -1.
func (q Quiz) Next() int {
	for i, it := range q.Items {
		if !it.Solved {
			return i
		}
	}
	return -1
}

// AnswerResult is what picking an option did.
type AnswerResult struct {
	Correct bool    `json:"correct"`
	Quiz    Quiz    `json:"quiz"`
	Outcome Outcome `json:"outcome"`
}

// Quiz picks are machine-local (.local/quiz.json): only finishing the quiz
// is synced, as the lesson's done event.
func (a *App) quizPath() string { return filepath.Join(a.Dir, LocalDir, "quiz.json") }

func (a *App) quizPicks() map[string][][]int {
	m := map[string][][]int{}
	if data, err := os.ReadFile(a.quizPath()); err == nil {
		json.Unmarshal(data, &m)
	}
	return m
}

func (a *App) saveQuizPicks(m map[string][][]int) error {
	p := a.quizPath()
	if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
		return err
	}
	data, _ := json.Marshal(m)
	tmp := fmt.Sprintf("%s.%d.tmp", p, os.Getpid())
	if err := os.WriteFile(tmp, data, 0o644); err != nil {
		return err
	}
	return os.Rename(tmp, p)
}

var errNotQuiz = errors.New("not a quiz")

// QuizState is a quiz lesson's questions with your progress. A quiz that's
// already done (perhaps on the other machine) shows every answer.
func (a *App) QuizState(l *course.Lesson) (Quiz, error) {
	if !l.IsQuiz() {
		return Quiz{}, errNotQuiz
	}
	qs, err := l.Quiz()
	if err != nil {
		return Quiz{}, err
	}
	return a.quizView(l, qs, a.quizPicks()[l.ID]), nil
}

func (a *App) quizView(l *course.Lesson, qs []course.Question, picks [][]int) Quiz {
	done := a.Progress.StatusOf(l.ID) == store.Done
	v := Quiz{Total: len(qs)}
	for i, q := range qs {
		it := QuizItem{Q: q.Q, Code: q.Code, Options: q.Options, Answer: -1}
		if i < len(picks) {
			it.Tried = picks[i]
		}
		if done || slices.Contains(it.Tried, q.Answer) {
			it.Solved, it.Answer, it.Why = true, q.Answer, q.Why
			v.Solved++
		}
		v.Items = append(v.Items, it)
	}
	return v
}

// AnswerQuiz picks option `pick` for question `q`. Solving the last question
// completes the lesson.
func (a *App) AnswerQuiz(l *course.Lesson, q, pick int) (AnswerResult, error) {
	if !l.IsQuiz() {
		return AnswerResult{}, errNotQuiz
	}
	qs, err := l.Quiz()
	if err != nil {
		return AnswerResult{}, err
	}
	if q < 0 || q >= len(qs) {
		return AnswerResult{}, fmt.Errorf("no question %d", q+1)
	}
	if pick < 0 || pick >= len(qs[q].Options) {
		return AnswerResult{}, fmt.Errorf("question %d has no option %d", q+1, pick+1)
	}
	if err := a.Start(l); err != nil {
		return AnswerResult{}, err
	}
	all := a.quizPicks()
	picks := all[l.ID]
	for len(picks) < len(qs) {
		picks = append(picks, nil)
	}
	if !slices.Contains(picks[q], qs[q].Answer) && !slices.Contains(picks[q], pick) {
		picks[q] = append(picks[q], pick)
	}
	all[l.ID] = picks
	if err := a.saveQuizPicks(all); err != nil {
		return AnswerResult{}, err
	}

	before := a.XP()
	res := AnswerResult{Correct: pick == qs[q].Answer}
	res.Quiz = a.quizView(l, qs, picks)
	if res.Quiz.Solved == res.Quiz.Total && a.Progress.StatusOf(l.ID) != store.Done {
		if err := a.SetStatus(l.ID, store.Done); err != nil {
			return res, err
		}
		res.Outcome.Completed = true
	}
	res.Outcome = a.outcome(before, res.Outcome.Completed)
	return res, nil
}

// ResetQuiz forgets your picks, so you can take the quiz again. It doesn't
// undo completing it.
func (a *App) ResetQuiz(l *course.Lesson) error {
	all := a.quizPicks()
	if _, ok := all[l.ID]; !ok {
		return nil
	}
	delete(all, l.ID)
	return a.saveQuizPicks(all)
}

// QuizModTime lets views notice answers given in another process.
func (a *App) QuizModTime() int64 {
	st, err := os.Stat(a.quizPath())
	if err != nil {
		return 0
	}
	return st.ModTime().UnixNano()
}
