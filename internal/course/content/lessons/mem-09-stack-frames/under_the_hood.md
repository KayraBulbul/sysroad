**What's in a frame.** On x86-64, `call` pushes the return address and
jumps. The callee usually pushes the caller's frame pointer (`rbp`),
then subtracts from the stack pointer (`rsp`) to make room for its
locals. `ret` pops the return address and jumps back. That's the whole
mechanism, and phase 4 has you read it in assembly.

**See frames in the debugger.** In gdb or lldb, `bt` lists the frames,
`frame 3` switches to one, and `info frame` (gdb) or `frame info`
(lldb) shows its addresses. A stack overflow's backtrace is thousands of
copies of the same function, which is the giveaway.

**Why ASan makes frames bigger.** ASan puts *redzones* around locals so
it can catch out-of-bounds writes on the stack. At `-O0` with ASan, a
frame with a couple of locals can be 200 bytes, so recursion runs out of
stack sooner than in an optimised build. Never rely on "it fits in an
optimised build". Inputs grow.

**Where did my local go?** Print `&local` under ASan and you may see a
heap-like address that goes *up*. To catch use of a local after its
function returned, ASan can move locals into a "fake stack" on the heap.
That's why `main.cpp` reads the frame pointer instead.

**The guard page.** Below the stack there's an unmapped *guard page*,
so running off the end faults instead of silently scribbling over other
memory. Threads get smaller stacks (8 MB on Linux, 512 KB on macOS for
secondary threads), which is why deep recursion in a worker thread
crashes sooner. Phase 8 comes back to this.

**Tail calls.** Some recursion compiles to a loop: if the recursive call
is the very last thing a function does, the optimiser can reuse the
frame. `sum_list` written as `return head->value + sum_list(head->next)`
isn't a tail call, because the addition happens after it returns. C++
never guarantees the optimisation anyway, so don't count on it.
