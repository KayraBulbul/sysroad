package course

import (
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
)

// Sanitizer reports are written for people debugging big programs: full
// paths, every frame down to _start, raw addresses and a dump of shadow
// memory. For a lesson, what matters is what went wrong and which lines of
// your code (and the tests) were involved, so tidySanitizer keeps those.

var (
	sanPidRE     = regexp.MustCompile(`==\d+==`)
	sanFrameRE   = regexp.MustCompile(`^(\s*)#\d+ 0x[0-9a-f]+ in (.*)$`)
	sanPcRE      = regexp.MustCompile(` at pc 0x[0-9a-f]+ bp 0x[0-9a-f]+ sp 0x[0-9a-f]+`)
	sanBuildIDRE = regexp.MustCompile(` \(BuildId: [0-9a-f]+\)`)
	sanDoctestRE = regexp.MustCompile(`\S*/doctest\.h`)
	sanBinRE     = regexp.MustCompile(` \(/\S+\+0x[0-9a-f]+\)`)
	sanAllocRE   = regexp.MustCompile(`, std::allocator<[^<>]*>>`)
)

// The spelled-out names of common standard types, and what you'd write.
var sanTypes = strings.NewReplacer(
	"std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>>", "std::string",
	"std::basic_string<char, std::char_traits<char>, std::allocator<char>>", "std::string",
	"std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>", "std::string",
	"std::basic_string_view<char, std::char_traits<char>>", "std::string_view",
	"std::__1::basic_string_view<char, std::__1::char_traits<char>>", "std::string_view",
	"[abi:cxx11]", "", "std::__1::", "std::",
)

// noiseFrame reports whether a stack frame is outside the learner's code:
// the standard library, the C runtime, doctest's runner, or sanitizer
// internals with no source line.
func noiseFrame(frame string) bool {
	for _, s := range []string{"/usr/", "/Library/Developer/", "/Applications/Xcode", "doctest.h",
		"libc_start", "_start (", "start (", "libsystem", "libclang_rt", "__asan", "__ubsan", "__sanitizer"} {
		if strings.Contains(frame, s) {
			return true
		}
	}
	// "operator new(unsigned long) (/path/to/binary+0x1234)": no source line.
	return strings.HasSuffix(frame, ")") && strings.Contains(frame, " (/") && strings.Contains(frame, "+0x")
}

// tidySanitizer shortens AddressSanitizer / UBSan output. work is the
// lesson's folder and testsDir where its test files were copied.
func tidySanitizer(s, work, testsDir string) string {
	if !strings.Contains(s, "Sanitizer") && !strings.Contains(s, "runtime error:") {
		return s
	}
	sep := string(filepath.Separator)
	var out []string
	skipping, frame := false, 0
	for _, line := range strings.Split(s, "\n") {
		if strings.HasPrefix(line, "Shadow bytes around the buggy address:") {
			skipping = true
			continue
		}
		if skipping {
			if strings.Contains(line, "ABORTING") {
				skipping = false
			}
			continue
		}
		if strings.Contains(line, "ABORTING") {
			continue
		}
		line = strings.ReplaceAll(line, testsDir+sep, "tests"+sep)
		line = strings.ReplaceAll(line, work+sep, "")
		line = sanDoctestRE.ReplaceAllString(line, "doctest.h")
		line = sanPidRE.ReplaceAllString(line, "")
		line = sanPcRE.ReplaceAllString(line, "")
		line = sanBuildIDRE.ReplaceAllString(line, "")
		short := func(s string) string {
			return sanAllocRE.ReplaceAllString(sanTypes.Replace(sanBinRE.ReplaceAllString(s, "")), ">")
		}
		if m := sanFrameRE.FindStringSubmatch(line); m != nil {
			if noiseFrame(m[2]) {
				continue
			}
			out = append(out, m[1]+"#"+strconv.Itoa(frame)+" "+short(m[2]))
			frame++
			continue
		}
		frame = 0
		out = append(out, short(line))
	}
	// No runs of blank lines.
	var tidy []string
	for i, l := range out {
		if strings.TrimSpace(l) == "" && i > 0 && strings.TrimSpace(out[i-1]) == "" {
			continue
		}
		tidy = append(tidy, l)
	}
	return strings.Join(tidy, "\n")
}
