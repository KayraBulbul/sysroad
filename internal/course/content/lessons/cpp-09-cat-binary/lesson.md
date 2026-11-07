---
title: Binary-safe copying
minutes: 10
files: [cat.cpp]
run:
  args: ["note.txt"]
  files:
    note.txt: "no newline at the end"
deeper:
  - title: std::basic_ostream::write (cppreference)
    url: https://en.cppreference.com/w/cpp/io/basic_ostream/write
---
Last lesson's `cat` has a bug. Try a file whose last line has no newline:

```text
$ printf 'no newline' > note.txt
$ ./cat note.txt | od -c
0000000   n   o       n   e   w   l   i   n   e  \n
```

There's a `\n` that wasn't in the file. `getline` throws the newline away
and your loop always adds one back, so it can't tell whether there was
one to begin with. For text that's mostly harmless. For a PNG, a `.o` file
or a tarball it's corruption, and `cat a.part b.part > whole.zip` has to
give back exactly the bytes it was given.

The fix is to stop thinking in lines and **copy bytes**:

```cpp
std::array<char, 65536> buf;
while (in.read(buf.data(), buf.size()) || in.gcount() > 0) {
    out.write(buf.data(), in.gcount());
}
```

`out.write(ptr, n)` writes exactly `n` bytes, zeros and all. `<<` on a
`const char*` would stop at the first `'\0'`. This is the same read loop
wc uses, now with a bigger buffer, because cat does nothing with the bytes
except pass them on.

`run_cat` already calls `copy_stream`. You only need to write it.

**The spec**

- `copy_stream` copies everything from `in` to `out`, byte for byte, and
  returns how many bytes it copied.

## Assignment

- [ ] Implement `copy_stream` with `read` and `write`
- [ ] Check a file with no trailing newline comes out unchanged
