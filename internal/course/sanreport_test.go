package course

import (
	"strings"
	"testing"
)

func TestTidySanitizerKeepsYourFrames(t *testing.T) {
	work, tests := "/home/me/.local/share/sysroad/work/x", "/home/me/.local/share/sysroad/.local/build/x/tests"
	in := `==4242==ERROR: AddressSanitizer: heap-use-after-free on address 0x7b0 at pc 0x55 bp 0x7ff sp 0x7fe
READ of size 4 at 0x7b0 thread T0
    #0 0x55ea in first_then_grow(std::vector<int>&) ` + work + `/bugs.cpp:17:12
    #1 0x55eb in DOCTEST_ANON_FUNC_5() ` + tests + `/visible_test.cpp:22:11
    #2 0x55ec in doctest::Context::run() /home/me/.cache/sysroad/doctest/abc/doctest.h:7013:21
    #3 0x7f4c in __libc_start_main /usr/src/debug/glibc/csu/libc-start.c:372:3
    #4 0x55ed in _start (/tmp/tests-run+0xa0424) (BuildId: 87d5)

freed by thread T0 here:
    #0 0x5590 in operator delete(void*) (/tmp/tests-run+0x19802c) (BuildId: 87d5)
    #1 0x5591 in std::vector<int>::_M_realloc_append(int&&) /usr/include/c++/16/bits/vector.tcc:12:1
    #2 0x5592 in first_then_grow(std::vector<int>&) ` + work + `/bugs.cpp:16:7

SUMMARY: AddressSanitizer: heap-use-after-free ` + work + `/bugs.cpp:17:12 in first_then_grow
Shadow bytes around the buggy address:
  0x7b6cc8ddfe00: 00 00 00 00
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
==4242==ABORTING`
	got := tidySanitizer(in, work, tests)
	want := `ERROR: AddressSanitizer: heap-use-after-free on address 0x7b0
READ of size 4 at 0x7b0 thread T0
    #0 first_then_grow(std::vector<int>&) bugs.cpp:17:12
    #1 DOCTEST_ANON_FUNC_5() tests/visible_test.cpp:22:11

freed by thread T0 here:
    #0 first_then_grow(std::vector<int>&) bugs.cpp:16:7

SUMMARY: AddressSanitizer: heap-use-after-free bugs.cpp:17:12 in first_then_grow`
	if strings.TrimSpace(got) != want {
		t.Fatalf("got:\n%s", got)
	}
	if plain := "just a normal failure\n"; tidySanitizer(plain, work, tests) != plain {
		t.Fatal("output without a sanitizer report must be left alone")
	}
}

func TestTidySanitizerShortensTypes(t *testing.T) {
	in := "ERROR: AddressSanitizer: x\n    #0 0x1 in welcome[abi:cxx11](std::basic_string_view<char, std::char_traits<char>>, std::vector<int, std::allocator<int>> const&) /w/bugs.cpp:31:12\n" +
		"SUMMARY: AddressSanitizer: heap-use-after-free (/tmp/b/tests-run+0x150d4f) in __asan_memcpy"
	want := "ERROR: AddressSanitizer: x\n    #0 welcome(std::string_view, std::vector<int> const&) bugs.cpp:31:12\n" +
		"SUMMARY: AddressSanitizer: heap-use-after-free in __asan_memcpy"
	if got := tidySanitizer(in, "/w", "/t"); got != want {
		t.Fatalf("got:\n%s", got)
	}
}
