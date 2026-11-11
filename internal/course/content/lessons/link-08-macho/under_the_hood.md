**Two-level namespaces.** On Linux, the loader looks every symbol up
in every library, in order, and takes the first match. Mach-O records
*which* library each symbol comes from (the `TWOLEVEL` flag in your
sample's header): `_puts` comes from `libSystem.B.dylib`, and dyld
won't look anywhere else. Two libraries can then export the same name
without clashing, at the cost of `LD_PRELOAD`-style interposition,
which needs `DYLD_INSERT_LIBRARIES` and explicit interpose sections.

**`@rpath` and friends.** An `LC_LOAD_DYLIB` name can start with
`@executable_path/`, `@loader_path/` or `@rpath/`, which dyld
replaces with the program's folder, the loading library's folder, or
each `LC_RPATH` in turn. That's how a `.app` bundle carries its own
libraries in `Contents/Frameworks` and still works wherever it's
dragged. `install_name_tool` rewrites these paths after linking.

**Code signing.** Every arm64 binary must be signed, or the kernel
won't run it. `LC_CODE_SIGNATURE` points into `__LINKEDIT` at a hash
of every 4 KB page; the kernel checks pages as they're loaded. A
linker signs with an *ad-hoc* signature (no identity, just the hashes)
by default, which is what `make mac-app` in sysroad itself does with
`codesign --sign -`. Patching a byte of a signed binary makes it fail
to launch until it's re-signed.

**Universal binaries.** The fat header is big-endian, a leftover from
NeXT and the PowerPC Macs, and shares its magic number with Java class
files: a count, then for each CPU its type, an offset and a size into
the file. `lipo -info /bin/ls` lists them, and `lipo -thin arm64`
extracts one. Apple Silicon Macs run the x86-64 slices of old
programs through Rosetta 2, which translates them to arm64 ahead of
time.

**Stubs instead of a PLT.** Your sample's `__stubs` and `__got`
sections play the PLT and GOT roles from lesson 5.3.4: `_main` calls a
stub, which jumps through a pointer in `__got` (or, for lazy binding,
`__la_symbol_ptr`) that dyld fills in with the address of `_puts`.
Newer binaries replace the binding opcodes of `LC_DYLD_INFO_ONLY`
with `LC_DYLD_CHAINED_FIXUPS`, a more compact encoding that makes
launching faster.
