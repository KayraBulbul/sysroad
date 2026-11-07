package tui

import (
	"fmt"
	"slices"
	"strings"

	tea "github.com/charmbracelet/bubbletea"
	"github.com/charmbracelet/x/ansi"

	"github.com/KayraBulbul/sysroad/internal/app"
	"github.com/KayraBulbul/sysroad/internal/course"
)

// Quizzes in the lesson panel: every question is shown, and 1–9 answers
// the first one that isn't solved yet.

func (v *lessonView) quizKey(c *core, l *course.Lesson, k string) (tea.Cmd, bool) {
	switch {
	case len(k) == 1 && k[0] >= '1' && k[0] <= '9':
		q, err := v.app.QuizState(l)
		if err != nil {
			c.fail(err)
			return nil, true
		}
		n := q.Next()
		if n < 0 {
			c.say("All answered — R to take it again")
			return nil, true
		}
		pick := int(k[0] - '1')
		if pick >= len(q.Items[n].Options) {
			c.say("Question %d has %d options", n+1, len(q.Items[n].Options))
			return nil, true
		}
		res, err := v.app.AnswerQuiz(l, n, pick)
		if err != nil {
			c.fail(err)
			return nil, true
		}
		c.changed()
		switch {
		case res.Outcome.Completed:
			c.say("%s", completedText(l, res.Outcome))
		case res.Correct:
			c.say("✓ Right — %d of %d", res.Quiz.Solved, res.Quiz.Total)
		default:
			c.say("✗ Not that one — try again")
		}
		v.jump = fmt.Sprintf("Q%d/", n+1) // keep the answer and its why in view
		return nil, true
	case k == "R":
		v.confirm = "Clear your answers and take the quiz again? (y/N)"
		v.onYes = func() tea.Cmd {
			c.fail(v.app.ResetQuiz(l))
			v.scroll = 0
			return nil
		}
		return nil, true
	case k == "r" || k == "s" || k == "v" || k == "e":
		c.say("This is a quiz — press 1–9 to answer")
		return nil, true
	}
	return nil, false
}

func (v *lessonView) quizLines(l *course.Lesson, w int) []string {
	q, err := v.app.QuizState(l)
	if err != nil {
		return []string{"", sErr.Render(wrap(err.Error(), w))}
	}
	var out []string
	add := func(s string) { out = append(out, strings.Split(s, "\n")...) }
	next := q.Next()
	for i, it := range q.Items {
		add("")
		head := fmt.Sprintf("Q%d/%d", i+1, q.Total)
		switch {
		case it.Solved:
			add(sGood.Render(head + " ✓"))
		case i == next:
			add(sAccent.Render(head) + sDim.Render(fmt.Sprintf("  press 1–%d", len(it.Options))))
		default:
			add(sDim.Render(head))
		}
		add(RenderMarkdown(it.Q, w))
		if it.Code != "" {
			add(strings.Join(codeBlock(strings.TrimRight(it.Code, "\n"), "cpp", w), "\n"))
		}
		for j, opt := range it.Options {
			label := fmt.Sprintf("%d) ", j+1)
			text := RenderMarkdown(opt, max(w-ansi.StringWidth(label)-2, 10))
			lines := strings.Split(text, "\n")
			mark, style := "  ", sDim
			switch {
			case it.Solved && j == it.Answer:
				mark, style = sGood.Render("✓ "), sGood
			case slices.Contains(it.Tried, j):
				mark, style = sErr.Render("✗ "), sErr
			}
			add(mark + style.Render(label) + lines[0])
			for _, rest := range lines[1:] {
				add(strings.Repeat(" ", 2+ansi.StringWidth(label)) + rest)
			}
		}
		if it.Solved && it.Why != "" {
			add(sDim.Render("  why: ") + strings.Join(strings.Split(RenderMarkdown(it.Why, max(w-7, 10)), "\n"), "\n       "))
		}
	}
	add("")
	if next < 0 {
		add(sGood.Render(fmt.Sprintf("✓ %d/%d — quiz complete", q.Solved, q.Total)) + sDim.Render("  · R: take it again"))
	} else {
		add(sDim.Render(fmt.Sprintf("%d/%d answered", q.Solved, q.Total)))
	}
	return out
}

// QuizText renders a quiz's questions for the CLI.
func QuizText(a *app.App, l *course.Lesson, w int) string {
	v := lessonView{app: a, id: l.ID}
	return strings.Join(v.quizLines(l, w), "\n")
}
