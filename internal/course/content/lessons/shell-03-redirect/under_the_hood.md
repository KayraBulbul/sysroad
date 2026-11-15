**Redirecting the shell itself.** `exec > log` (no command) applies a
redirection to the shell permanently, and every later command
inherits it: the classic way for a script to log everything it does.
`exec 3< file` opens a file on fd 3 for the rest of the script, and
`read -u 3` reads from it.

**What tokens really are.** A real shell's grammar has more operators
(`<<`, `<<-`, `<>`, `>|`, `<<<`, `&>`) and a rule that the fd number
must touch the operator: `2 > f` is the word `2` and a redirection of
stdout. Your lexer gets that right, because the digits are only read
as an fd when `<` or `>` follows straight after.

**The tokenizer-parser split.** Lexing turns characters into tokens,
and parsing turns tokens into structure. Keeping them separate is how
every compiler is built, and you'll do it properly in phase 11's
interpreter. A shell's lexer is unusually hard, because what's an
operator depends on quotes, and a word's meaning on where it sits.
