**util-linux seeks, you read.** For `-s`, real hexdump calls `fseeko` to
jump straight to the offset, which is instant on a 10 GB file. Pipes can't
seek, so `cat file | hexdump -s 16` fails with `Illegal seek`. Your
version reads and throws bytes away: slower on huge files, but it works on
anything. `dd skip=` and `tail -c +N` have the same trade-off.

**`std::from_chars` is the fast parser.** It doesn't allocate, doesn't
look at the locale, and doesn't throw, and it tells you exactly where it
stopped. That's how `parse_number` rejects `12abc`: the parse stops
before the end. `std::stoi` would throw, and `atoi` would silently
return 12.

**Comparing 16-byte lines is cheap.** `bytes == prev_` is a length check
plus `memcmp`, which compilers turn into two 8-byte loads per side. Phase
5 looks at what `memcmp` actually compiles to.

**Squeezing shows structure.** Dump a fresh disk image or a core file,
and the `*` lines show you where the real data is. Executables have long
runs of zero padding between sections, aligned to the page size, which
you'll meet again in the linking chapter.
