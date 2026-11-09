---
title: "The stack, frame by frame"
minutes: 20
files: [walk.cpp]
run: {}
deeper:
  - title: Call stack (Wikipedia)
    url: https://en.wikipedia.org/wiki/Call_stack
  - title: "Stack frames in gdb (Debugging with GDB)"
    url: https://sourceware.org/gdb/current/onlinedocs/gdb.html/Frames.html
---
Every function call gets a **frame** on the stack: room for its locals,
the saved registers it will restore, and the address to jump back to
when it returns. Calling pushes a frame and returning pops it, so
allocating a local costs nothing more than moving the stack pointer.

On x86-64 and ARM the stack **grows down**: each new frame sits at a
lower address than its caller's. `main.cpp` prints where each of a few
nested calls puts its frame, so you can watch it happen:

```
frame 0 is at 0x7ffd07f46350
frame 1 is at 0x7ffd07f46320, 48 bytes below its caller
frame 2 is at 0x7ffd07f462f0, 48 bytes below its caller
```

The catch is size. The main thread's stack is usually **8 MB**
(`ulimit -s` shows it in KB). Recursion that goes a million calls deep
needs a million frames, which is far more than 8 MB. The program runs
past the end of the stack into an unmapped page and crashes. ASan
reports it as `stack-overflow`, and without ASan you get a plain
`Segmentation fault`.

So for data that can be arbitrarily deep, like a linked list or a
lopsided tree, swap the call stack for a loop, plus a stack of your own
on the heap when you need to remember where you were:

```cpp
std::vector<const Tree*> todo = {root};   // your stack, on the heap
while (!todo.empty()) {
    const Tree* t = todo.back();
    todo.pop_back();
    // ...visit t, then push its children
}
```

`walk.cpp` has recursive versions that work on small inputs. The tests
feed them a million nodes.

## Assignment

- [ ] `sum_list` adds up a linked list with a loop instead of recursion
- [ ] `tree_depth` finds the deepest path with an explicit stack of (node, depth) pairs
- [ ] `count_nodes` counts every node in a tree without recursing
