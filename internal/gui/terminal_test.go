package gui

import (
	"os"
	"os/exec"
	"strings"
	"testing"
)

// The script a terminal window runs for an interactive lesson: the program
// starts in the lesson folder with the engine's sanitizer settings, and its
// exit code is shown. On macOS it goes through a .command file that cleans
// itself up.
func TestTerminalScriptRunsTheProgram(t *testing.T) {
	dir := t.TempDir()
	prog := exec.Command("sh", "-c", `echo "in $(pwd)"; echo "asan=$ASAN_OPTIONS"; exit 3`)
	prog.Dir = dir
	prog.Env = append(os.Environ(), "ASAN_OPTIONS=detect_leaks=1", "UNRELATED=1")

	path, err := commandFile(terminalScript(prog))
	if err != nil {
		t.Fatal(err)
	}
	// No terminal here: stdin is empty, so the closing "press Enter" read
	// fails, which is the script's exit status; the output is what counts.
	out, err := exec.Command(path).CombinedOutput()
	if _, exited := err.(*exec.ExitError); err != nil && !exited {
		t.Fatal(err)
	}
	for _, want := range []string{"in " + dir, "asan=detect_leaks=1", "[exit 3]"} {
		if !strings.Contains(string(out), want) {
			t.Errorf("output lacks %q:\n%s", want, out)
		}
	}
	if _, err := os.Stat(path); !os.IsNotExist(err) {
		t.Error("the .command file wasn't removed")
	}
}
