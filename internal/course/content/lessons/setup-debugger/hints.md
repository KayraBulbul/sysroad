For question 2, find the line number of `acc = acc * 31 + x % 1000;` in
your editor, then `break drill.cpp:<that line> if i == 37`, `run`,
`print acc`. It stops *before* running that line, which is what the
question asks for.
---
For questions 3 and 4, just `run` with no breakpoints. It stops at the
crash and shows the function, its arguments and the line. `bt` shows the
whole chain from `main`.
---
Typing `printf` debugging into `drill.cpp` would also work, but this
lesson is the moment to get the debugger into your hands. You'll need it
for the bomb lab in phase 4, where there's no source code at all.
