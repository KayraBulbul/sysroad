---
title: Variables and quoting
kind: capstone
program: conch
minutes: 60
files: [lexer.cpp, expand.cpp]
run:
  interactive: true
deeper:
  - title: "Quoting, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_02
  - title: "Word expansions, in the POSIX shell spec"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/V3_chap02.html#tag_19_06
  - title: "BashPitfalls: what word splitting does to people"
    url: https://mywiki.wooledge.org/BashPitfalls
---
So far `echo 'a b'` prints `'a b'`, quotes and all. This milestone
conch learns quoting and variables, in two steps that real shells
keep apart.

**Step one, in the lexer: find the words.** Quotes and backslashes
decide where a word ends, but nothing is removed or replaced yet.
Inside `'...'`, `"..."`, or after a `\`, spaces don't end the word
and `|`, `;`, `&`, `<`, `>` aren't operators. The token keeps the word
exactly as written: `echo "a | b"` gives the words `echo` and `"a | b"`,
with the quotes. Inside double quotes, `\"` doesn't end them. An
unclosed quote is a `SyntaxError`. While you're there, mark a word
`assignment` if it starts with a name (a letter or `_`, then letters,
digits and `_`) followed by `=`, all unquoted: `X=1`, `A_B='x y'`.

**Step two, just before the command runs: expand each word.** The
course calls your `expand_word` on every word, assignment value and
redirection target, at the moment the pipeline runs, so `false; echo
$?` sees the `false`. It returns the word with:

```text
'text'      exactly as written, no exceptions
"text"      $ expansions happen; \ only escapes " \ $ and `, else it stays
\c          the character c, whatever it is
$NAME ${NAME}   the variable: a shell variable, else the environment, else ""
$?  $$  $!      last status, the shell's PID, the last background job's PID
~  ~/...        $HOME, only at the very start of an unquoted word
$               not followed by any of those: just a $
```

A word with no quotes and no plain text that expands to nothing (say
`$UNSET`) isn't a word at all: return `nullopt`. `""` is an empty word.
Unlike sh, conch doesn't split a variable's value into several words:
`X='a b'; printf [%s] $X` prints `[a b]`, as zsh does.

`vars.cpp` (the course's) keeps **shell variables**: `X=1` alone sets
one, which commands don't see until you `export X`. `X=1 cmd` puts `X`
in `cmd`'s environment only. In front of a builtin, as in bash, it
lasts for that builtin (`HOME=/ cd`), except for `export`, `unset` and
`exit`, where it stays (`X=1 export X`).

## Assignment

- [ ] `tokenize` keeps quoted and backslashed text in one word, as written, throws for an unclosed quote, and marks assignments
- [ ] `expand_word` handles single quotes, double quotes and backslashes
- [ ] It expands `$NAME`, `${NAME}`, `$?`, `$$`, `$!` and a leading `~`
- [ ] A word that expands to nothing unquoted vanishes; a quoted empty word stays
