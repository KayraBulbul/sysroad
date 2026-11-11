**What `execve` does with this.** The kernel reads the ELF header and
the program headers, and nothing else. It `mmap`s each `LOAD` segment
at its `p_vaddr` with its flags (adding a random offset for a
position-independent executable, `DYN`, which is what `/bin/ls` is),
zeroes the part of `p_memsz` past `p_filesz`, and sets up the stack
with the arguments and environment. If there's an `INTERP` segment,
it also loads the program it names, the dynamic loader, and starts
*that* instead of `e_entry`. The loader then loads the shared
libraries, does the relocations, and jumps to `e_entry`.

**Your sample has no loader.** `sample_x86_64_exe` was linked
statically with no C library, so it has no `INTERP` and the kernel
jumps straight to `_start` at `0x201160`. Its three `LOAD` segments
are read-only data, code and writable data, each starting on a new
4 KB page in memory, so each can get its own permissions. Its
`GNU_STACK` segment says the stack shouldn't be executable: an old
security fix that's become a header field.

**Why `/bin/ls` starts at address 0.** A position-independent
executable is linked as if it would load at 0, and the kernel picks a
random base each run (address space layout randomisation). That's why
its type is `DYN`: to the kernel, a PIE is just a shared library that
happens to have an entry point. (Newer `readelf`s print
`DYN (Position-Independent Executable file)`, after finding a `PIE`
flag in the dynamic section.) Run
`cat /proc/self/maps` twice and the addresses differ.

**`e_shnum` can lie.** If a file has 65,280 sections or more, the
count doesn't fit in 16 bits, so `e_shnum` is 0 and the real count is
in the `sh_size` field of section 0. `e_shstrndx` has the same escape
hatch. Huge C++ object files (one section per inline function and
template) do hit this. A tool that parses ELF without checking every
offset and count is a security bug waiting to happen, which is why
fuzzers find so many bugs in object-file readers.

**`readelf` and `objdump`.** GNU `readelf` was written to parse ELF
on its own, without the BFD library that `objdump` and `ld` use, so
it could serve as an independent check on them. `llvm-readelf` and
`llvm-objdump` are LLVM's versions, and they also read Mach-O, so
they work on the Mac.
