package course

import (
	"fmt"

	"gopkg.in/yaml.v3"
)

// Question is one multiple-choice question from a quiz lesson's quiz.yaml.
type Question struct {
	Q       string   `yaml:"q"`              // markdown
	Code    string   `yaml:"code,omitempty"` // optional C++ shown under the question
	Options []string `yaml:"options"`        // markdown, one line each
	Answer  int      `yaml:"answer"`         // index into Options
	Why     string   `yaml:"why"`            // markdown, shown once it's answered right
}

// IsQuiz reports whether the lesson is a written multiple-choice quiz.
// Answering every question right completes it.
func (l *Lesson) IsQuiz() bool {
	if !l.Written || l.Kind != KindQuiz {
		return false
	}
	_, ok := l.read("quiz.yaml")
	return ok
}

// Quiz is the lesson's questions, in order.
func (l *Lesson) Quiz() ([]Question, error) {
	s, ok := l.read("quiz.yaml")
	if !ok {
		return nil, nil
	}
	var qs []Question
	if err := yaml.Unmarshal([]byte(s), &qs); err != nil {
		return nil, fmt.Errorf("%s/quiz.yaml: %w", l.ID, err)
	}
	return qs, nil
}

func (l *Lesson) validateQuiz() []string {
	qs, err := l.Quiz()
	if err != nil {
		return []string{err.Error()}
	}
	if len(qs) == 0 {
		return []string{l.ID + ": a quiz needs questions in quiz.yaml"}
	}
	var errs []string
	for i, q := range qs {
		where := fmt.Sprintf("%s/quiz.yaml question %d", l.ID, i+1)
		switch {
		case q.Q == "":
			errs = append(errs, where+": empty question")
		case len(q.Options) < 2:
			errs = append(errs, where+": needs at least two options")
		case len(q.Options) > 9:
			errs = append(errs, where+": at most nine options (they're picked with 1–9)")
		case q.Answer < 0 || q.Answer >= len(q.Options):
			errs = append(errs, where+": answer is out of range")
		case q.Why == "":
			errs = append(errs, where+": explain the answer in why")
		}
	}
	return errs
}
