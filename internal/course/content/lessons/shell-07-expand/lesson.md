---
title: Quoting and expansion
kind: capstone
project: conch
program: conch
minutes: 50
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
So far `echo 'a b'` prints `'a b'`, quotes and all. This lesson conch
learns quoting and `$`, in two steps that real shells keep apart.

**Step one, in the lexer: find the words.** Quotes and backslashes
decide where a word ends, but nothing is removed or replaced yet.
Inside `'...'`, `"..."`, or after a `\`, spaces don't end the word,
`#` doesn't start a comment, and `|`, `;`, `&`, `<`, `>` aren't
operators. The word keeps its text exactly as written: `echo "a | b"`
gives the words `echo` and `"a | b"`, with the quotes. Inside double
quotes, `\"` doesn't end them. An unclosed quote is a syntax error.

**Step two, just before the command runs: expand each word.** Every
word, and every redirection's target, is expanded at the moment its
pipeline runs, so `false; echo $?` sees the `false`. Expanding a word
gives the word with:

```text
'text'          exactly as written, no exceptions
"text"          $ expansions happen; \ only escapes " \ $ and `, else it stays
\c              the character c, whatever it is
$NAME ${NAME}   the variable's value, or nothing if it isn't set
$?  $$  $!      the last status, the shell's PID, the last background job's PID
~  ~/...        $HOME, only at the very start of an unquoted word
$               not followed by any of those: just a $
```

A name is a letter or `_`, then letters, digits and `_`, as long as
they go: `$N_` is the variable `N_`, and `$N.` is `N` then a dot. For
now a variable is an environment variable (`getenv`); next lesson adds
the shell's own. A `${` with no `}` is a syntax error, found when the
command runs.

A word with no quotes and no plain text that expands to nothing (say
`$UNSET`) isn't a word at all, and disappears; `""` is an empty word,
and stays. A command whose words all disappear runs nothing. Unlike
sh, conch doesn't split a variable's value into several words:
`printf [%s] $X`, with `X` set to `a b`, prints `[a b]`, as zsh does.

The job table and `jobs` keep a job's text as it was written, quotes
and all.

## Contract

Nothing new.

## Assignment

- [ ] Quoted and backslashed text stays in one word, isn't an operator or a comment, and an unclosed quote is a syntax error
- [ ] Words lose their quotes and backslashes when they run: single quotes keep everything, double quotes expand `$`, and `\` escapes only `" \ $` and `` ` `` inside them
- [ ] `$NAME`, `${NAME}`, `$?`, `$$`, `$!` and a leading `~` expand when the command runs, in its words and its redirection targets
- [ ] A word that's only unset variables disappears; a quoted empty word stays
