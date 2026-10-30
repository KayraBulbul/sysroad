package roadmap

import "testing"

func TestEveryDefaultStepHasDoneWhen(t *testing.T) {
	r, err := Parse(DefaultYAML)
	if err != nil {
		t.Fatal(err)
	}
	for _, s := range r.Sections {
		for _, st := range s.Steps {
			if st.DoneWhen == "" {
				t.Errorf("step %s (%s) has no done_when", st.ID, st.Title)
			}
		}
	}
}

func TestUpgradeKeepsUserStructure(t *testing.T) {
	user := &Roadmap{Sections: []*Section{{ID: "a", Title: "A", Steps: []*Step{
		{ID: "mine", Title: "added by user"},
		{ID: "a-1", Title: "old", Detail: "old detail"},
	}}}}
	builtin := &Roadmap{Sections: []*Section{
		{ID: "a", Title: "A", Steps: []*Step{{ID: "a-1", Title: "new", Detail: "new detail", DoneWhen: "x"}}},
		{ID: "b", Title: "B", Steps: []*Step{{ID: "b-1"}}},
	}}

	if ch := user.Upgrade(builtin, false); len(ch) != 1 || ch[0].ID != "a-1" || len(ch[0].Fields) != 3 {
		t.Fatalf("dry run changes = %+v", ch)
	}
	if user.Sections[0].Steps[1].Title != "old" {
		t.Fatal("dry run modified the roadmap")
	}
	user.Upgrade(builtin, true)
	steps := user.Sections[0].Steps
	if len(user.Sections) != 1 || len(steps) != 2 || steps[0].ID != "mine" {
		t.Fatal("upgrade changed the user's structure")
	}
	if steps[1].Title != "new" || steps[1].Detail != "new detail" || steps[1].DoneWhen != "x" {
		t.Fatalf("step not upgraded: %+v", steps[1])
	}
	if ch := user.Upgrade(builtin, false); len(ch) != 0 {
		t.Fatalf("second upgrade should be a no-op, got %+v", ch)
	}
}
