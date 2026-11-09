**Why 8?** Tab stops every 8 columns go back to typewriters and the
first terminals, and it's what `cat` and `less` show. Editors let you
set it, and a project's `.editorconfig` can say what it wants, but
files on disk can't say how wide their tabs are, which is the root of
the tabs-against-spaces argument.

**Two strings per line.** Keeping `render` costs memory, but drawing
and scrolling never re-expand tabs. Later lessons add a third array,
the colour of each render character. Big editors use cleverer
structures (gap buffers, ropes, piece tables) so that inserting into
a huge file doesn't copy much, but a vector of lines is what kilo,
early vi and many others used, and it's fine for files up to tens of
megabytes.

**Line endings.** kite drops the `\r` and will save with `\n`, so a
Windows file changes on save. Careful editors remember the file's line
ending and write it back. git's `core.autocrlf` exists for the same
reason.

**`std::getline` and binary files.** A NUL byte or invalid UTF-8 goes
through untouched: `getline` only cares about `\n`. The renderer will
draw control characters as they are, which a terminal may act on. The
syntax highlighting lesson shows them safely instead.
