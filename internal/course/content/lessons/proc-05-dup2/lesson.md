---
title: "File descriptors, dup2 and redirection"
minutes: 40
files: [redirect.cpp]
program: redir
run:
  args: ["<names.txt", "2>&1", "sort"]
  files:
    names.txt: "carol\nalice\nbob\n"
deeper:
  - title: "CS:APP 10.8-10.9: sharing files, and I/O redirection"
    url: https://csapp.cs.cmu.edu/
  - title: "dup(2) man page"
    url: https://man7.org/linux/man-pages/man2/dup.2.html
  - title: "Redirection, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_07
---
`sort < names.txt > sorted.txt` works without `sort` knowing anything
about files: it reads fd 0 and writes fd 1, like always. The shell
arranged for 0 and 1 to *be* those files before it ran `sort`. That's
redirection, and it happens in the gap between `fork` and `exec`.

It helps to know what an fd points at. There are three levels:

```text
your fd table        open file (kernel)            the file
 0 ─────────────▶  ┌─────────────────────┐
 1 ─────────────▶  │ offset 1024, O_RDWR │ ───▶  inode: /dev/pts/3
 2 ─────────────▶  └─────────────────────┘
 3 ─────────────▶  ┌─────────────────────┐
                   │ offset 0, O_RDONLY  │ ───▶  inode: names.txt
                   └─────────────────────┘
```

Your fds are just numbers in a per-process table. Each points at an
**open file**: the kernel's record of one `open` call, holding the
current offset and the mode. Several fds can point at the same open
file, and so can fds in different processes: after a `fork`, parent
and child share every open file, offsets included.

`dup2(old, new)` makes fd `new` point at the same open file as `old`,
closing whatever `new` pointed at first. So redirecting stdout to a
file is:

```cpp
int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
dup2(fd, 1);   // 1 now points at out.txt
close(fd);     // don't leave the extra fd around
```

Shells give you five forms, each with an optional fd number in front:

```text
[n]<path     open path for reading as fd n (0 if no n)
[n]>path     open it for writing, empty it first (fd 1 if no n)
[n]>>path    open it for appending
[n]>&m       make fd n a copy of fd m: 2>&1 sends stderr wherever stdout goes
[n]<&m       the same, with 0 as the default
```

They apply **left to right**, and each `>&` copies where an fd points
*now*. So `cmd > log 2>&1` sends both to `log`, but `cmd 2>&1 > log`
sends stderr to where stdout pointed before, the terminal, and only
stdout to `log`. People get that one wrong for years.

`redir` takes redirections as words and then a command:
`redir "<names.txt" ">out.txt" sort`. You do the redirecting in the
child, after `fork` and before `execvp`, so the parent's fds stay as
they were.

## Assignment

- [ ] `parse_redirect` reads all five forms with an optional fd number, and returns `nullopt` for any other word, including one with no path or a bad fd number
- [ ] `apply_redirect` opens the file with the right flags (mode `0666`), `dup2`s it into place and closes the extra fd, or just `dup2`s for `>&` and `<&`
- [ ] It throws `std::system_error` with the path (or the fd `m`) as its message when something fails, including a `>&m` whose `m` isn't open
- [ ] `run_redirected` applies the redirections in the child, in order; if one fails the child prints `redir: ` and the error, and exits 1
