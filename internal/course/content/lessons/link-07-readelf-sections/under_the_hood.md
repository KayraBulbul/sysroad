**Where `/bin/ls`'s symbols went.** Distributions *strip* their
binaries (`strip`, or `-s` at link time): the `.symtab` and `.strtab`
sections are removed, because nothing needs them to run, and they
can be half the file. What's left is `.dynsym`, the **dynamic symbol
table**: only the symbols the dynamic loader needs, which are the
functions `/bin/ls` imports from libc. `readelf --dyn-syms /bin/ls`
shows them. Debuggers find the rest in separate debug files, fetched
by the build ID in `.note.gnu.build-id` (that's what `debuginfod`
does).

**Why locals come first.** In every ELF symbol table, the `LOCAL`
symbols come before the others, and the symbol table's `sh_info` is
the index of the first non-local one (4 in the sample). The linker
only needs to look at symbols from there on when resolving names
across files.

**Sections in a `.o` start at 0.** In the object file, `add` has value
0 and `bump` 0x11: offsets within `.text`. Only after linking do they
get addresses, like `_start` at `0x201160` in the executable. That
move is what the relocations in `.rela.text` let the linker make: for
each place in the code that refers to a symbol, an entry saying which
symbol, and how to patch the bytes once its address is known.

**Section symbols.** Entry 3 in the sample is a `SECTION` symbol with
no name: it stands for `.bss` itself. The relocation for `hidden`
refers to it ("`.bss` plus 0"), because a local symbol can't be
looked up by name from outside the file anyway, and section symbols
keep the table small.

**DWARF.** Build with `-g` and the object gains `.debug_info`,
`.debug_line`, `.debug_str` and friends, which describe types,
variables and which address came from which line. That's what `gdb`
reads to show you source. They're often bigger than everything else
in the file put together, which is another reason to strip release
builds.
