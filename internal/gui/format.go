package gui

import (
	"bytes"
	"context"
	"errors"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

// CourseStyle is the clang-format style the course's own code is written
// in, used when there's no .clang-format above the lesson folder.
const CourseStyle = "{BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 100, PointerAlignment: Left, " +
	"AllowShortFunctionsOnASingleLine: Empty, SpacesBeforeTrailingComments: 2}"

// Format runs clang-format over a lesson file's content (the editor's, which
// may not be saved yet) and returns the result. A .clang-format in the lesson
// folder or above it, e.g. in the data folder, wins over the course style.
func (s *Service) Format(id, name, content string) (string, error) {
	s.mu.Lock()
	l, err := s.editable(id, name)
	var work string
	if err == nil {
		work = s.app.Engine.WorkDir(l)
	}
	s.mu.Unlock()
	if err != nil {
		return "", err
	}
	if !isCxxFile(name) {
		return content, nil // CMakeLists.txt and such: clang-format would mangle them
	}
	bin, err := exec.LookPath("clang-format")
	if err != nil {
		hint := "sudo pacman -S clang"
		if runtime.GOOS == "darwin" {
			hint = "brew install clang-format"
		}
		return "", errors.New("clang-format isn't installed (" + hint + ")")
	}
	style := CourseStyle
	if hasClangFormat(work) {
		style = "file"
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	cmd := exec.CommandContext(ctx, bin, "--style="+style, "--assume-filename="+filepath.Join(work, name))
	cmd.Stdin = strings.NewReader(content)
	var out, stderr bytes.Buffer
	cmd.Stdout, cmd.Stderr = &out, &stderr
	if err := cmd.Run(); err != nil {
		if msg := strings.TrimSpace(stderr.String()); msg != "" {
			return "", errors.New("clang-format: " + msg)
		}
		return "", err
	}
	return out.String(), nil
}

// hasClangFormat reports whether clang-format would find a style file for
// files in dir: a .clang-format or _clang-format there or in a parent.
func hasClangFormat(dir string) bool {
	for d := dir; ; d = filepath.Dir(d) {
		for _, n := range []string{".clang-format", "_clang-format"} {
			if _, err := os.Stat(filepath.Join(d, n)); err == nil {
				return true
			}
		}
		if filepath.Dir(d) == d {
			return false
		}
	}
}

// isCxxFile reports whether clang-format should touch the file.
func isCxxFile(name string) bool {
	switch strings.ToLower(filepath.Ext(name)) {
	case ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".ipp", ".inl":
		return true
	}
	return false
}
