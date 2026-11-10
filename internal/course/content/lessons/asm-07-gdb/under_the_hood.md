**How a breakpoint works.** The debugger overwrites the first byte of
the instruction with `int3` (the byte `cc`) on x86-64, or the whole
instruction with `brk` on arm64. When the CPU reaches it, it traps
into the kernel, which stops the process and tells the debugger. To
carry on, the debugger puts the original bytes back, runs that one
instruction, and plants the breakpoint again.

**How single-stepping works.** The CPU has a flag for it: with the
*trap flag* set, it traps after every instruction. `stepi` sets the
flag and resumes. The debugger does all of this through one system
call, `ptrace`, which lets one process read and write another's
registers and memory.

**Why `strings` didn't give phase 1 away.** `strings vault` prints
every run of printable bytes in a file, and a secret kept as a plain
string literal shows up immediately. The vault keeps the word
XOR-ed with a key and decodes it on the stack just before comparing.
That stops `strings` and does nothing against a debugger: the plain
text has to exist in memory at the moment of the comparison. Every
"hidden" key in a program you can run has the same weakness.

**Addresses change between runs.** Modern systems load a program at a
random address each time (ASLR), so an attacker can't know where
things are. gdb turns that off for the program it runs, which is why
the addresses in `disassemble` are the same every time, and why
`break *0x…` keeps working after `run`. lldb does the same.

**Useful extras.** In gdb, `display/i $pc` shows the next instruction
after every step, `layout asm` and `layout regs` split the terminal
into live views, and `set disassembly-flavor intel` switches syntax.
`watch *(long*)0x…` stops when a memory location changes. In lldb,
`register read --all`, `watchpoint set expression`, and `gui` for a
similar split view.

**This is the bomb lab in miniature.** The CS:APP bomb is a program
with six phases and no source, and it "explodes" on a wrong answer.
You'll use exactly these commands, plus one habit: put a breakpoint
on the function that explodes before you run anything.
