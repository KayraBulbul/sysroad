**Readline's tricks.** GNU Readline has emacs keys by default and vi
keys after `set -o vi`, a kill ring (Ctrl-Y pastes what Ctrl-W or
Ctrl-K cut), Ctrl-R incremental history search, tab completion that
asks the shell for candidates, and `~/.inputrc` to rebind everything.
It's about 30,000 lines. linenoise covers what most people use in
about 1,100.

**Wide characters and the cursor.** conch counts bytes, so a line
with `é` (two bytes in UTF-8) or a CJK character (two columns wide)
puts the cursor in the wrong place. Real editors measure each
character's display width with `wcwidth`, and redraw from the prompt
when a line wraps past the terminal's width, which `TIOCGWINSZ`
reports and `SIGWINCH` announces.

**What you've built.** conch has the core of every Unix shell:
tokenizing, parsing, fork and exec, redirection, pipelines, `&&` and
`||`, builtins, background jobs, job control with process groups and
the terminal, quoting and expansion, and line editing. What a real
shell adds is mostly more grammar: `if`, `for`, `while`, functions,
`$(...)`, globbing and here-documents. Each is a weekend once you have
this.
