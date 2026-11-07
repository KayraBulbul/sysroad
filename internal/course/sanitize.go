package course

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"time"
)

// baseFlags are used for every build: C++20, debug info, no optimisation.
var baseFlags = []string{"-std=c++20", "-g", "-O0", "-fno-omit-frame-pointer", "-fcolor-diagnostics"}

// sanitizerSets are tried in order; a machine uses the first one whose test
// program builds and runs. Sanitizers make memory bugs and undefined
// behaviour fail loudly instead of "working".
var sanitizerSets = []struct {
	Name  string
	Flags []string
}{
	{"AddressSanitizer + UBSan", []string{"-fsanitize=address,undefined", "-fno-sanitize-recover=undefined"}},
	{"UBSan only", []string{"-fsanitize=undefined", "-fno-sanitize-recover=undefined"}},
	{"no sanitizers", nil},
}

const probeTimeout = 8 * time.Second

// ProbeRow is one sanitizer set's result in the toolchain check.
type ProbeRow struct {
	Name   string `json:"name"`
	OK     bool   `json:"ok"`
	Detail string `json:"detail"`
}

type sanitizerChoice struct {
	Index int        `json:"index"` // into sanitizerSets
	Rows  []ProbeRow `json:"rows"`
}

var probeMu sync.Mutex

const probeSource = `#include <iostream>
#include <span>
#include <string>
#include <vector>
int main() {
    std::vector<int> v{1, 2, 3};
    std::span<int> s{v};
    std::string msg = "hello from C++20";
    std::cout << msg << ' ' << s.size() << '\n';
}
`

func (e *Engine) probeKey(ctx context.Context, cxx string) string {
	ver, _ := exec.CommandContext(ctx, cxx, "--version").Output()
	osver, _ := exec.CommandContext(ctx, "uname", "-r").Output()
	h := sha256.New()
	fmt.Fprint(h, cxx, string(ver), string(osver), runtime.GOOS, runtime.GOARCH, len(sanitizerSets))
	return hex.EncodeToString(h.Sum(nil))[:16]
}

func (e *Engine) probePath(key string) string {
	return filepath.Join(e.CacheDir, "sanitizers", key+".json")
}

// probe builds and runs a small program with each sanitizer set.
func (e *Engine) probe(ctx context.Context, cxx string) sanitizerChoice {
	choice := sanitizerChoice{Index: -1}
	tmp, err := os.MkdirTemp("", "sysroad-probe-*")
	if err != nil {
		return sanitizerChoice{Index: len(sanitizerSets) - 1}
	}
	defer os.RemoveAll(tmp)
	src := filepath.Join(tmp, "hello.cpp")
	os.WriteFile(src, []byte(probeSource), 0o644)
	for i, set := range sanitizerSets {
		bin := filepath.Join(tmp, fmt.Sprintf("hello%d", i))
		args := append(append(append([]string{}, baseFlags...), set.Flags...), src, "-o", bin)
		row := ProbeRow{Name: set.Name}
		if res := compile(ctx, tmp, cxx, args); res.err != nil {
			row.Detail = "doesn't build: " + firstLine(stripANSI(string(res.out)))
		} else {
			run := e.exec(ctx, bin, nil, "", nil, probeTimeout)
			switch {
			case run.timedOut:
				row.Detail = fmt.Sprintf("builds, but the program hangs (killed after %s)", probeTimeout)
			case run.code != 0:
				row.Detail = fmt.Sprintf("builds, but the program exits %d: %s", run.code, firstLine(stripANSI(run.stderr)))
			case !strings.Contains(run.stdout, "hello from C++20 3"):
				row.Detail = fmt.Sprintf("builds, but prints %q", firstLine(run.stdout))
			default:
				row.OK = true
				row.Detail = fmt.Sprintf("builds and runs (%dms)", run.took.Milliseconds())
			}
		}
		choice.Rows = append(choice.Rows, row)
		if row.OK && choice.Index < 0 {
			choice.Index = i
		}
		if i == 0 && row.OK {
			break // the full set works; no need to try weaker ones
		}
	}
	if choice.Index < 0 {
		choice.Index = len(sanitizerSets) - 1
	}
	return choice
}

// sanitizers returns this machine's sanitizer flags, probing once per
// compiler and OS version. $SYSROAD_SANITIZE (address, undefined or none)
// overrides it. note explains a fallback.
func (e *Engine) sanitizers(ctx context.Context, cxx string, force bool) (flags []string, note string, choice sanitizerChoice) {
	switch os.Getenv("SYSROAD_SANITIZE") {
	case "address":
		return sanitizerSets[0].Flags, "", sanitizerChoice{}
	case "undefined":
		return sanitizerSets[1].Flags, "", sanitizerChoice{}
	case "none":
		return nil, "", sanitizerChoice{}
	}
	probeMu.Lock()
	defer probeMu.Unlock()
	key := e.probeKey(ctx, cxx)
	p := e.probePath(key)
	loaded := false
	if !force {
		if data, err := os.ReadFile(p); err == nil && json.Unmarshal(data, &choice) == nil && choice.Index >= 0 && choice.Index < len(sanitizerSets) {
			loaded = true
		}
	}
	if !loaded {
		choice = e.probe(ctx, cxx)
		if os.MkdirAll(filepath.Dir(p), 0o755) == nil {
			data, _ := json.Marshal(choice)
			tmp := fmt.Sprintf("%s.%d.tmp", p, os.Getpid())
			if os.WriteFile(tmp, data, 0o644) == nil {
				os.Rename(tmp, p)
			}
		}
	}
	if choice.Index > 0 && len(choice.Rows) > 0 {
		note = fmt.Sprintf("Using %s on this machine: %s %s. The toolchain check has details.",
			sanitizerSets[choice.Index].Name, sanitizerSets[0].Name, choice.Rows[0].Detail)
	}
	return sanitizerSets[choice.Index].Flags, note, choice
}

// flags is everything a build passes: base flags plus this machine's sanitizers.
func (e *Engine) flags(ctx context.Context, cxx string) ([]string, string) {
	san, note, _ := e.sanitizers(ctx, cxx, false)
	return append(append([]string{}, baseFlags...), san...), note
}
