package roadmap

import "testing"

func TestValidateRejectsDuplicateIDs(t *testing.T) {
	r := &Roadmap{Sections: []*Section{
		{ID: "a", Steps: []*Step{{ID: "x"}}},
		{ID: "b", Steps: []*Step{{ID: "x"}}},
	}}
	if err := r.Validate(); err == nil {
		t.Fatal("duplicate step ids should fail validation")
	}
}

func TestReadOnlyAndGroupAreNotSaved(t *testing.T) {
	r := &Roadmap{Sections: []*Section{{ID: "a", Title: "A", Group: "G", ReadOnly: true}}}
	data, err := Marshal(r)
	if err != nil {
		t.Fatal(err)
	}
	back, err := Parse(data)
	if err != nil {
		t.Fatal(err)
	}
	if back.Sections[0].ReadOnly || back.Sections[0].Group != "" {
		t.Fatalf("runtime-only fields leaked into YAML:\n%s", data)
	}
}
