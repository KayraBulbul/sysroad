**Where the sequences come from.** The VT100 sent `ESC [ A` for Up, or
`ESC O A` when a program switched its keypad to "application mode".
Terminals still do both, which is why Home has so many spellings: each
terminal family made its own choices for the keys the VT100 didn't
have. `ESC [ 1 ; 5 C` is Ctrl-Right: the `5` after the `;` says which
modifiers were held (1 plus 4 for Ctrl).

**Escape is ambiguous, for real.** On a slow ssh link, the bytes of
`\x1b[A` can arrive far enough apart that kite takes the ESC for an
Escape press and then types `[A`. Vim's `ttimeoutlen` and tmux's
`escape-time` are this tenth of a second, made configurable. Newer
protocols fix it at the source: kitty's keyboard protocol sends Escape
itself as a sequence, so a lone ESC never happens.

**Alt is ESC.** Most terminals send Alt-x as ESC then `x`. That's why
kite reads `\x1bx` as Escape followed by `x`: it has no use for Alt.
Emacs reads the same two bytes as `M-x`.

**Paste.** Pasting into a terminal types the text, all at once. A
pasted ESC character would start a "key", which is one reason
terminals offer *bracketed paste* (`\x1b[?2004h`): the paste arrives
between `\x1b[200~` and `\x1b[201~`, so a program knows it's text.
