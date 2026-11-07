---
title: "grep: fixed strings"
minutes: 15
files: [grep.cpp]
run:
  args: ["error", "app.log", "worker.log"]
  files:
    app.log: "start\nerror: disk full\nretrying\nerror: still full\n"
    worker.log: "ok\nok\n"
deeper:
  - title: grep(1) man page
    url: https://man7.org/linux/man-pages/man1/grep.1.html
---
The project for this chapter is `grep`: print the lines that contain a
pattern. Real grep's patterns are regular expressions. Yours are **fixed
strings** (like `grep -F`), which is what most searches are anyway:

```text
$ grep error app.log
error: disk full
error: still full
$ grep error app.log worker.log
app.log:error: disk full
app.log:error: still full
```

With more than one file, each line gets its file's name in front. Stdin
(no files, or `-`) is called `(standard input)`.

**grep's exit status means something.** Scripts use it in `if grep -q
...`:

- 0: at least one line matched
- 1: no line matched
- 2: something went wrong (a missing file, a directory, no pattern).
  This beats 0: even if other files matched, an error gives 2.

Everything else is what you've built before: `std::getline` for lines,
`find` for the match, and `std::ifstream`, `is_directory` and
`std::strerror(errno)` for the files.

**The spec**

- `grep_stream` prints `prefix + line + '\n'` for each line containing
  `pattern`, and returns how many. An empty pattern matches every line.
- `run_grep`: the first argument is the pattern, the rest are files. With
  no arguments at all, write `Usage: grep [OPTION]... PATTERNS [FILE]...`
  to `err` and return 2.
- Errors go to `err` as `grep: NAME: No such file or directory` and
  `grep: NAME: Is a directory`.

## Assignment

- [ ] Implement `grep_stream`
- [ ] Implement `run_grep`: pattern, files or stdin, name prefixes
- [ ] Report errors and return 0, 1 or 2
