**Why x86-64 register names are such a mess.** They're fifty years of
history. The 8-bit 8080 had a register `a`. The 16-bit 8086 widened it
to `ax`, the 32-bit 386 to `eax` ("extended"), and the 64-bit design
from AMD to `rax`. All the old names still work and name the low part
of the same register: `al` is the low byte of `ax`, inside `eax`,
inside `rax`. When AMD doubled the register count they gave up on
letters and called the new ones `r8` to `r15`.

**arm64 started clean.** It was designed in 2011 as a new instruction
set, not as an extension of the 32-bit one, so it has 31 numbered
registers, and every instruction is exactly 4 bytes. x86-64 instructions are 1 to 15 bytes
long. That fixed size is why an arm64 instruction can't hold a 64-bit
constant, and why you'll see big constants built in pieces.

**Writing a 32-bit register clears the top half.** On both machines,
an instruction that writes `eax` or `w0` sets the upper 32 bits of the
full register to 0. That's why `fifth` needs no extra instruction to
return an `int`.

**Two operands or three.** x86-64's `addq %rsi, %rax` overwrites one
of its inputs, so the compiler often needs a `movq` first to save a
value. arm64's `add x0, x1, x2` keeps both inputs, so it needs fewer
copies. `leaq` is popular on x86-64 partly because it's the one
arithmetic instruction with a separate destination.

**Intel syntax.** The same x86-64 instructions have a second
spelling, where the destination comes first and there are no `%` or
`$` signs: `lea rax, [rdi + 4*rsi]` and `add rax, 10`. Compiler
Explorer shows Intel syntax by default, and gdb and clang print AT&T.
The next lesson shows how to switch each tool to the one you like.

**Machine code is just bytes.** `retq` is the single byte `c3`.
`addq $10, %rax` is `48 83 c0 0a`: a prefix that says "64-bit", the
opcode, a byte naming the register, and the constant 10. An
*assembler* does this translation, and a *disassembler* such as
`objdump -d` turns the bytes back into text.
