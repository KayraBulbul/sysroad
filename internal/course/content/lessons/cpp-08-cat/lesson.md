---
title: cat one file, then many
minutes: 15
files: [cat.cpp]
run:
  args: ["a.txt", "nope.txt", "b.txt"]
  files:
    a.txt: "first file\n"
    b.txt: "second file\nline two\n"
deeper:
  - title: std::getline (cppreference)
    url: https://en.cppreference.com/w/cpp/string/basic_string/getline
---
`cat` ("concatenate") prints its files one after another:

```text
$ cat a.txt b.txt
first file
second file
line two
```

With no arguments, or the argument `-`, it copies stdin. That's why
`echo hi | cat` works.

This chapter builds GNU's `cat`, flags and all. The first version reads
**line by line**:

```cpp
std::string line;
while (std::getline(in, line)) {
    out << line << '\n';
}
```

`std::getline` reads up to the next `'\n'`, stores the line *without* the
newline, and returns the stream, which tests false once nothing is left. So
the loop puts the newline back itself. Keep that in mind: next lesson finds
the bug it causes.

**Two errors to report**, both on `err`, and then carry on:

- A file that can't be opened: `cat: NAME: No such file or directory`, from
  `std::strerror(errno)` like in wc.
- A directory: `cat: NAME: Is a directory`. Opening a directory for reading
  *succeeds* on Linux and only the reads fail, so check first with
  `std::filesystem::is_directory`.

Return 1 if anything failed, else 0.

## Assignment

- [ ] Implement `cat_lines` with `std::getline`
- [ ] Implement `run_cat`: stdin, `-`, files, and both errors
