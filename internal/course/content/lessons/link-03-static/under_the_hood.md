**Why the GNU index is big-endian.** The format comes from early Unix
on machines like the VAX and Sun, and was frozen as is. The BSD index
is in the byte order of the machine that made it, little-endian on
every Mac since Apple left PowerPC. Formats designed later, like ELF,
say their byte order in the header (lesson 5.3.5).

**Circular dependencies between libraries.** If `liba.a` needs
`libb.a` and `libb.a` needs `liba.a`, one left-to-right pass isn't
enough. GNU `ld` lets you bracket them with `--start-group liba.a
libb.a --end-group`, which rescans the group until nothing new is
pulled in. Or list one library twice. `lld` and Apple's `ld` scan all
the archives as needed, which is why order mistakes only show up on
some machines.

**Thin archives.** `ar rcT` makes a GNU *thin* archive (magic
`!<thin>\n`): the index and the names, with paths to the object files
instead of copies. Big builds like Chromium and LLVM use them to save
disk and time.

**`-ffunction-sections` and `--gc-sections`.** Pulling in a member
pulls in all of it, even the functions you don't call. Compiling with
`-ffunction-sections -fdata-sections` puts every function and global
in its own section, and linking with `-Wl,--gc-sections` (`-Wl,-dead_strip`
on the Mac) then drops every section nothing refers to. Embedded
projects and Rust use this to shrink binaries.

**Static vs shared.** A statically linked program carries its own copy
of everything it uses: no dependencies at run time, the same behaviour
on every machine, and a bigger file. Go and Rust programs are mostly
static for that reason. Shared libraries, next lesson, make the
opposite trade.
