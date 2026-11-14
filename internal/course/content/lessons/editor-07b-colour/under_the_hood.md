**Colours, then and now.** The 8 colours (`30`–`37`) and their bright
versions (`90`–`97`) are what every terminal can show, and your theme
decides what "red" looks like. `\x1b[38;5;208m` picks from a fixed
256-colour palette, and `\x1b[38;2;255;128;0m` is 24-bit "true colour",
which the theme can't change. Editors that bring their own colour
schemes use true colour; kite follows your terminal's.

**Drawing only what changed.** kite writes a colour code each time the
colour changes, which on a busy line is every few characters. Real
terminals parse that fast, but ncurses-era programs went further: they
kept a copy of the screen, characters and attributes, and sent only
the cells that differ from the last frame. That's what mattered on a
9600-baud line, where every byte took a millisecond.

**Why control characters are dangerous.** A file with `\x1b]0;...\x07`
in it changes your terminal's title when you `cat` it, and old
terminals had sequences that typed text back into the input, which is
an exploit if someone can get bytes into a log you'll read. `less`
shows them as `^[` for the same reason, and `cat -v` (lesson 1.2.5)
does it on request.

**Keeping colours next to the text.** A colour per character costs
memory, but it means drawing never has to think about the language.
Editors with big files do the opposite and keep colours as runs
("columns 4 to 9 are a string"), which take less space and are what
language servers send.
