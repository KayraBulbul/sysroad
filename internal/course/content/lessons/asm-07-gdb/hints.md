Phase 1 on the PC:

```text
gdb --args ./vault hello 1 2
(gdb) break strcmp
(gdb) run
(gdb) x/s $rdi
(gdb) x/s $rsi
```

`strcmp` takes two pointers, so they're in the first two argument
registers. One is your `hello`. On the Mac it's `b strcmp`, `run`,
then `x/s $x0` and `x/s $x1`.
---
Phase 2: start again with the right word, or phase 1 stops the
program before phase 2 runs.

```text
gdb --args ./vault THE-WORD 1 2
(gdb) break phase_2
(gdb) run
(gdb) disassemble
(gdb) nexti          <- repeat until the arrow is just past the call to forge
(gdb) p $rax
```

A function's result is in `rax` (`x0` on the Mac) right after the
call returns. The next instructions compare it with your number.
`break forge`, `run`, `finish` gets you to the same place.
---
Phase 3: look at the table first. On the PC, `x/8dg &table` prints
its eight numbers. On the Mac, the same eight numbers are in
`vault-arm64.s` under `_table:`, or stop in `phase_3` after the
`adrp`/`add` pair has run and use `x/8dg $x9`.

Then read `phase_3`. It refuses an index above 7. Otherwise it does
`index = table[index]` three times and checks whether the result
is 5.
---
Work phase 3 backwards. Which entry of the table holds 5? Call its
position `a`. Which entry holds `a`? Call that `b`. Which entry holds
`b`? That last position is the index to give. Check it in the
debugger: `break phase_3`, run with your index, and `stepi` through
the loop while watching `rdi` (`x0`) change.
