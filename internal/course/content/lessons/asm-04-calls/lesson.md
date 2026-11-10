---
title: Calling conventions and the stack
minutes: 30
files: [mystery.cpp]
deeper:
  - title: "System V AMD64 ABI: the x86-64 calling convention (OSDev wiki)"
    url: https://wiki.osdev.org/System_V_ABI
  - title: "Writing ARM64 code for Apple platforms (Apple)"
    url: https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms
  - title: "Stack frame layout on x86-64 (Eli Bendersky)"
    url: https://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64
---
Every function uses the same few registers. So when one function
calls another, they need an agreement about who may overwrite what.
That agreement is the **calling convention**. Lesson 2.2.1 showed you
stack frames from the outside. This is how they're built.

**The stack.** One register always points at the top of the stack:
`rsp` on x86-64, `sp` on arm64. The stack grows *down*, so making
room means subtracting. `pushq %rbx` subtracts 8 from `rsp` and
stores `rbx` there. `popq %rbx` loads it back and adds 8.
`8(%rsp)` on x86-64 and `[sp, #8]` on arm64 both mean "the memory 8
bytes above the top".

**Call and return.** A call has to remember where to come back to:

- x86-64: `callq f` pushes the address of the next instruction, the
  **return address**, and jumps to `f`. `retq` pops it and jumps back.
- arm64: `bl f` puts the return address in the register `x30` and
  jumps. `ret` jumps to whatever is in `x30`. A function that makes
  calls of its own must save `x30` first, or it loses its way home.

**Who saves what.** A call is allowed to destroy some registers and
must preserve others:

- x86-64: `rbx`, `rbp` and `r12` to `r15` survive a call. Everything
  else may come back changed, including all the argument registers.
- arm64: `x19` to `x28` survive. `x0` to `x18` may not.

So a value that's needed after a call gets parked in a surviving
register. And since the caller's caller is counting on that register
too, the function saves the old value on the stack first and restores
it before returning:

```cpp
long shifted(long a, long b) { return square(a) + b; }
```

```
shifted:                         x86-64
    pushq   %rbx                 save the caller's rbx
    movq    %rsi, %rbx           park b where the call can't touch it
    callq   square@PLT           rax = square(a); a was already in rdi
    addq    %rbx, %rax           rax += b
    popq    %rbx                 put rbx back
    retq
```

```
_shifted:                        arm64
    stp     x20, x19, [sp, #-32]!    sp -= 32; save x20 and x19
    stp     x29, x30, [sp, #16]      save x29 and the return address
    add     x29, sp, #16
    mov     x19, x1                  park b
    bl      _square                  x0 = square(a)
    add     x0, x0, x19              x0 += b
    ldp     x29, x30, [sp, #16]      restore everything
    ldp     x20, x19, [sp], #32      and sp += 32
    ret
```

`stp` stores a pair of registers and `ldp` loads a pair. The `!`
means "move `sp` first", and a number after the brackets means "move
it afterwards". The saves at the top and the restores at the bottom
are bookkeeping. Once you recognise them, skip them and read the
middle.

**More than six arguments.** x86-64 passes the first six in
registers. The seventh and later go on the stack, and at the start of
the function they're at `8(%rsp)`, `16(%rsp)` and so on, just above
the return address at `(%rsp)`. arm64 has eight argument registers,
`x0` to `x7`.

**Locals.** A local array needs an address, so it lives on the stack.
`subq $40, %rsp` reserves 40 bytes, and `movq %rsp, %rdi` passes
their address as the first argument.

The listings have five functions. They call `square` and `fill`,
which are in `helpers.cpp`.

## Assignment

- [ ] `foxtrot`: eight arguments, and it uses three of them
- [ ] `golf`: one call
- [ ] `hotel`: two calls, with a value kept across the first
- [ ] `india`: a function that calls itself
- [ ] `juliet`: a local array that another function fills in
