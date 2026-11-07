**Directory walking is `getdents64`.** `recursive_directory_iterator`
opens each directory and reads its entries in batches with the
`getdents64` system call. The order is whatever the filesystem stores:
ext4 uses a hash order, APFS sorts by name. That's the nondeterminism you
avoided by sorting.

**ripgrep and friends.** `rg` (ripgrep) is grep -r taken seriously: it
walks the tree in parallel threads, skips `.gitignore`d files and binary
files by default, and uses SIMD for literal search. It's a great codebase
to read after the concurrency phase.

**Why grep checks for NUL bytes.** Text files almost never contain a zero
byte, and binary files almost always do, so it's a cheap and reliable
test. GNU grep also looks for encoding errors in UTF-8 locales. `-a`
(treat everything as text) and `--binary-files=without-match` change the
behaviour.

**Symlink loops.** `a/link -> ..` makes an infinite tree. Not following
symlinks avoids loops completely. `-R` follows them, and remembers each
directory's (device, inode) pair to notice when it's been somewhere
before, which is also how `find -L` copes.
