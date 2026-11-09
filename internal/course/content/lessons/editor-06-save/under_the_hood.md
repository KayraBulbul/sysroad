**Is rename enough?** Not quite. The rename is atomic, but the data
may still be in the kernel's cache when it happens. After a power cut,
some filesystems have come back with the new name pointing at an empty
file. The full recipe is: write, `fsync` the file, rename, then `fsync`
the folder. `std::ofstream` can't `fsync`, so careful programs use
POSIX `open`/`write`/`fsync` for this. Dan Luu's article in Deeper is
a tour of how hard this gets.

**What rename loses.** The new file is a new file: it gets default
permissions, the saver as its owner, and any hard links to the old file
keep pointing at the old contents. vim's `backupcopy` option chooses
between renaming (safe) and overwriting in place (keeps those). kite
could copy the old file's mode with `stat` and `chmod` before the
rename.

**Swap files.** vim also writes your unsaved changes to a `.swp` file
every few seconds, so a crash loses almost nothing, and it warns if a
second vim opens the same file.

**`dirty` as a counter.** kilo counts changes rather than setting a
flag. Either works. The undo lesson makes it subtle: undo back to
the saved state, and the file isn't really modified any more. Editors
that get this right remember which point in the undo history was saved.
