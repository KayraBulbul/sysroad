**Reading hex dumps is a core skill.** A file starting `7f 45 4c 46` is
an ELF executable (`\x7fELF`), `89 50 4e 47` is a PNG, `50 4b 03 04` is
a zip, and `cf fa ed fe` is a Mach-O binary on your Mac. Phase 5's mini
readelf starts exactly like this: dump the first 64 bytes, then decode
them.

**Little-endian shows up here.** Write the 32-bit number 1 to a file and
hexdump shows `01 00 00 00`, lowest byte first, because x86-64 and arm64
both store numbers little-endian. Phase 4 (bits and bytes) uses this a lot.

**`xxd` is the other common tool.** `xxd file` prints a similar layout,
and `xxd -r` turns a hex dump back into bytes, which makes it a crude
binary editor: dump, edit in your text editor, convert back.

**hexdump is really a formatter.** `-C` is shorthand for a format string:
`hexdump -e '"%08.8_ax  " 8/1 "%02x " "  " 8/1 "%02x " "  |" 16/1 "%_p" "|\n"'`.
The `-e` language can print almost any fixed layout, which is how people
decode packed binary records from the shell.
