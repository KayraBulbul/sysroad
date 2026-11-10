Question 1: the first label is `_ZN3geo4areaEii` (with one more
underscore on the Mac). Paste it after `c++filt` in a terminal. On the
Mac, give it the name exactly as the listing has it.
---
Question 2: `banner` loads the address of a label into the register
that holds the return value. Search for that label further down. The
`.asciz` line under it is the text.
---
Question 3: the source had a loop, and the listing has none. The
compiler ran the loop itself and kept only the result. It's the
constant that `answer` moves into `eax` (or `w0`).
---
Question 4: follow `bump` line by line. It loads `counter`, stores
that plus 1 back, and returns the *loaded* value plus something.
`counter`'s starting value is the `.quad` under its label.
---
Question 5: `greet` puts a string's address in the first-argument
register and then jumps to a function. The `@PLT` on Linux and the
leading underscore on the Mac aren't part of its name.
