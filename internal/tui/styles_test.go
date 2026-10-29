package tui

import "testing"

func TestReflow(t *testing.T) {
	cases := map[string]string{
		"a b\nc d":                       "a b c d",
		"a length-\nprefixed thing":      "a length-prefixed thing",
		"p1 line\nmore\n\np2":            "p1 line more\n\np2",
		"- item one\n- item two":         "- item one\n- item two",
		"Mac:     brew\nArch:    pacman": "Mac:     brew\nArch:    pacman",
		"intro:\n  indented":             "intro:\n  indented",
	}
	for in, want := range cases {
		if got := reflow(in); got != want {
			t.Errorf("reflow(%q) = %q, want %q", in, got, want)
		}
	}
}

func TestWindowKeepsCursorVisible(t *testing.T) {
	for n := 0; n < 30; n++ {
		for cur := 0; cur < n; cur++ {
			s, e := window(n, cur, 7)
			if cur < s || cur >= e || e-s > 7 || s < 0 || e > n {
				t.Fatalf("window(%d,%d,7) = %d,%d", n, cur, s, e)
			}
		}
	}
}
