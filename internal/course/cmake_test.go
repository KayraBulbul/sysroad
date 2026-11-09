package course

import "testing"

func TestGeneratorSkipsMultiConfig(t *testing.T) {
	for _, g := range []string{"Ninja Multi-Config", "Xcode", "Visual Studio 17 2022"} {
		t.Setenv("CMAKE_GENERATOR", g)
		if got := generator(); got == g {
			t.Errorf("generator() kept multi-config %q", g)
		}
	}
	t.Setenv("CMAKE_GENERATOR", "Unix Makefiles")
	if got := generator(); got != "Unix Makefiles" {
		t.Errorf("generator() = %q, want Unix Makefiles", got)
	}
}
