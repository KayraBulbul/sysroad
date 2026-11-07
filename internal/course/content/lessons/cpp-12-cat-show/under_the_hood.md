**Caret notation is the Ctrl key.** Pressing Ctrl+A in a terminal sends
byte 1, Ctrl+M sends 13 (`\r`, which is why Enter can be Ctrl+M), and
Ctrl+[ sends 27, ESC. Each one is the letter's code minus 64, which is
what `^` means. You'll use this directly in the text editor capstone, where
raw mode hands you these bytes one key at a time.

**M- is the old Meta key.** Some 1970s keyboards had a Meta key that set
the top bit of the byte. Today those bytes are mostly UTF-8, where every
byte of a multi-byte character is 128 or above. That's why `cat -v` turns
`é` (`C3 A9`) into `M-CM-)`.

**Finding `\r\n` files.** A script with Windows line endings fails with
`/bin/bash^M: bad interpreter`, and `cat -A script.sh | head` shows the
`^M` right away. `dos2unix` or `sed -i 's/\r$//'` fixes it.

**Trailing whitespace.** `cat -E file | grep ' \$$'` finds lines that end in a
space. Editors and `git diff` highlight the same thing for you.
