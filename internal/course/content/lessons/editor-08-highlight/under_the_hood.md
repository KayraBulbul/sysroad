**This isn't parsing.** kite looks at one line at a time with a
handful of rules, so it's fooled easily: a raw string
`R"(" )"` confuses it, `std::string` is "Normal" while `int` is a
type, and a `'` digit separator (`1'000'000`) opens a string. kilo,
nano and early vim all work like this, and it's right most of the time
for almost no code.

**Real highlighters.** vim and Sublime use regular expressions per
language, with states for strings and comments, much like yours. VS
Code uses TextMate grammars, the same idea at a larger scale. Neovim,
Helix and Zed use Tree-sitter, which really parses the file and
re-parses only the edited part, so it knows a type from a variable.
Language servers (clangd) add "semantic tokens" on top: actual compiler
knowledge of what each name is.

**Why a comment needs the rows after it.** Typing `/*` on line 10
changes the colour of everything until the next `*/`, possibly
thousands of lines. The course re-highlights following rows only while
the "starts inside a comment" state keeps changing, which is why each
row remembers how it started (`hl_in`). That's kilo's trick, and it
makes the common case, an edit that doesn't open or close a comment,
cost one row.

**Colours.** kite uses the 16 palette colours (`30`–`37`), so it
follows your terminal's theme. `\x1b[38;5;208m` picks from 256 colours,
and `\x1b[38;2;255;128;0m` is 24-bit "true colour", which the theme
can't change.
