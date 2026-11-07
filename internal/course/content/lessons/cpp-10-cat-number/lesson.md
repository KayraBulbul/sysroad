---
title: "-n and -b: numbering lines"
minutes: 15
files: [cat.cpp]
run:
  args: ["-b", "poem.txt"]
  files:
    poem.txt: "so much depends\nupon\n\na red wheel\nbarrow\n"
---
`cat -n` numbers lines, and `cat -b` numbers only the ones that aren't
empty:

```text
$ cat -b poem.txt
     1	so much depends
     2	upon

     3	a red wheel
     4	barrow
```

Each number is right-aligned in 6 columns and followed by a **tab**:
`std::format("{:>6}\t", n)`. GNU's other choices are worth copying too:

- Numbering **continues across files**. `cat -n a b` doesn't restart at 1.
- If a file doesn't end in a newline, its last line runs into the next
  file's first line, and that joined line gets one number.
- `-b` wins over `-n` when you give both.

**State that outlives a call.** Since numbering continues across files,
the "where am I" facts can't be local variables in `cat_stream`. They live
in a `CatState` that `run_cat` makes once and passes to every call by
reference:

```cpp
struct CatState {
    std::size_t line = 0;       // the last number printed
    bool at_line_start = true;  // the next byte starts a line
    int blank_run = 0;          // for -s, next lesson
};
```

`cat_stream` goes through the input one byte at a time. Whenever a byte
arrives while `at_line_start` is true, that's the start of a new line,
and the place to write the number. Building the output in a `std::string`
per chunk and writing it once keeps it fast.

The starter has the rest of `cat` ready: every flag is parsed already,
`plain()` skips straight to `copy_stream` when no flag is given, and
`run_cat` keeps one `CatState` for all the files. `-s`, `-E`, `-T` and `-v`
do nothing until the next two lessons.

## Assignment

- [ ] At the start of each line, write the number for `-n`
- [ ] Make `-b` skip empty lines, and win over `-n`
- [ ] Keep `st.at_line_start` up to date
