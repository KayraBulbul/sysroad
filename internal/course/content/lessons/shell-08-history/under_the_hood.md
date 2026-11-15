**The history is a list of strings.** bash's history file is exactly
that, one line per command (with `HISTTIMEFORMAT`, a `#` timestamp line
before each). `!!` runs the last line again, `!42` line 42, and `!gcc`
the last line starting with `gcc`: history expansion, done on the line
before anything else, which is why `echo "hi!"` surprises people in
bash.

**`HISTCONTROL`.** conch skips a repeat of the last line, as bash does
with `HISTCONTROL=ignoredups`. bash's `ignorespace` also skips lines
that start with a space, a way to keep a command with a password in it
out of the history.

**Why only interactive shells save history.** A script that saved its
commands would fill your history with them, and two shells writing one
file would lose each other's lines. bash appends at exit (`shopt -s
histappend`) and can write after every command (`PROMPT_COMMAND='history
-a'`); zsh can share one history file live between all its windows.

**rename and the file's owner.** Saving through a new file and `rename`
makes a new file, with the saver as its owner and the mode it was
created with, which is what you want here: whatever mode an old file
had, the new one is `0600`. The flip side: a symlink at
`~/.conch_history` is replaced by a plain file, not followed.

