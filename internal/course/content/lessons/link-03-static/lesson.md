---
title: Static libraries
minutes: 45
files: [archive.cpp]
program: artool
run: {}
deeper:
  - title: "CS:APP 7.6.2-7.6.3: linking with static libraries"
    url: https://csapp.cs.cmu.edu/
  - title: "ar (file format), Wikipedia"
    url: https://en.wikipedia.org/wiki/Ar_(Unix)
  - title: "llvm-ar: archive formats"
    url: https://llvm.org/docs/CommandGuide/llvm-ar.html
---
A **static library** (`libfoo.a`) is nothing more than object files in
a bag, the format `ar` has used since the 1970s:

```sh
clang++ -c add.cpp mul.cpp
ar rcs libmath.a add.o mul.o      # r: add or replace, c: create, s: index
clang++ main.o -L. -lmath -o prog
```

`-lmath` makes the linker look for `libmath.a` in the `-L` folders.
What it does with it is the interesting part. It doesn't copy the
whole library in. It keeps a list of symbols that are still
undefined, and pulls in only the **members that define one of them**,
then adds whatever those members need in turn. A program that calls
only `add` gets `add.o` and never `mul.o`. That's why libc is split
into hundreds of tiny object files, one function each.

It also explains the most confusing link error there is. GNU `ld`
reads the command line **left to right**, and only looks in a library
for symbols that are undefined *at that moment*. Put the library
first (`clang++ -lmath main.o`) and nothing is undefined yet, so it
pulls nothing, and then `main.o` comes along needing `add`:
`undefined reference to 'add'`. Libraries go after the code that uses
them. (Apple's linker and `lld` are forgiving about the order.)

To find which member defines a symbol without opening every one, the
archive starts with a **symbol index**, built by `ranlib` or the `s`
in `ar rcs`. You'll read both the members and the index. The format:

```text
"!<arch>\n"                                      8 bytes, then members:
name[16] date[12] uid[6] gid[6] mode[8] size[10] "`\n"     a 60-byte header
data[size]                                       then '\n' if size is odd
```

Every field is ASCII text padded with spaces; `size` is in decimal.
The name field has variants:

- **GNU** (Linux): a short name ends with `/`: `add.o/`. The member
  named `/` is the symbol index, and the one named `//` holds the long
  names, each ending `/\n`. A member named `/34` has the long name
  starting at offset 34 in that table.
- **BSD** (the Mac): a name like `#1/20` means the real name is the
  first 20 bytes of the data, padded with `\0`s, and the rest of the
  data is the file. The index is a member named `__.SYMDEF` (or
  `__.SYMDEF SORTED`).

The two indexes:

```text
GNU "/":          count (4 bytes, big-endian), then count member header offsets
                  (4 bytes each, big-endian), then count names, each ending '\0'
BSD "__.SYMDEF":  n (4 bytes, little-endian), then n / 8 pairs of
                  (name offset, member header offset), 4 bytes each, little-endian,
                  then the string table's size (4 bytes) and the string table
```

Both point at a member by the offset of its *header* in the file, so
remember where each header started.

On the Mac, libraries that come with Xcode are often **universal**,
with an archive per CPU inside (`lipo -info libfoo.a` says). Take one
out with `lipo -thin arm64 libfoo.a -output thin.a` before reading it.

## Assignment

- [ ] Check the magic, then walk the headers, keeping members at even offsets
- [ ] Short names, GNU long names (`/n`) and BSD long names (`#1/n`)
- [ ] Read the GNU or BSD symbol index into `symbols`
- [ ] Throw `std::runtime_error` for anything cut short or malformed
- [ ] Make a `.a` of your own with `ar rcs` and read it with `artool`
