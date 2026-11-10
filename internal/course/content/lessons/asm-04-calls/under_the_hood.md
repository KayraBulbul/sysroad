**Where this agreement comes from.** It's called an ABI, an
*application binary interface*. Linux, macOS and the BSDs on x86-64
all follow the System V AMD64 ABI. Windows uses a different one, with
arguments in `rcx`, `rdx`, `r8`, `r9`. arm64 has AAPCS64, with a few
Apple changes. The ABI is why code from clang, gcc, Rust and Go's
cgo can call each other: they agree on registers, not on languages.

**Why 16-byte alignment.** Some instructions that move 16 bytes at
once (the SSE ones that `printf` uses for floating point, for
instance) crash if their memory operand isn't on a 16-byte boundary.
So the convention makes `rsp` a multiple of 16 at every call. The
`callq` then pushes 8 bytes, which is why a function that only needs
to realign pushes one junk register. Hand-written assembly that
forgets this crashes inside `printf`, far from the mistake. Keep it
in mind for the attack lab.

**The frame pointer.** `rbp` and `x29` traditionally point at a fixed
place in the current frame, with the caller's saved frame pointer
there, forming a linked list of frames up the stack. Debuggers and
profilers can walk it cheaply. At `-O1` and up, clang on Linux stops
using `rbp` that way and uses it as one more ordinary register.
Apple requires `x29` to be kept, which is why every arm64 function
here saves it. The course's builds pass `-fno-omit-frame-pointer`
so ASan's stack traces are fast.

**The return address is just data on the stack.** `retq` jumps to
whatever 8 bytes are at the top. If a function writes past the end of
a local array, it can overwrite them, and then `retq` goes wherever
the overwriting bytes say. That is the classic *buffer overflow*
attack, and it's what you'll do on purpose in the attack lab. arm64
keeps the return address in `x30`, but any function that makes a call
has saved it to the stack, so the same attack works.

**The red zone.** On x86-64 Linux and macOS, a function that calls
nothing may use the 128 bytes *below* `rsp` without moving it. That
saves two instructions in small functions. Kernels turn it off,
because an interrupt would write over it.

**Floating point goes elsewhere.** `double` and `float` arguments
travel in a separate set of registers, `xmm0` to `xmm7` on x86-64 and
`d0` to `d7` on arm64, and a floating-point result comes back in
`xmm0` or `d0`.
