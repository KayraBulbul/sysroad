**Member functions are ordinary functions underneath.** The compiler
passes the object's address as a hidden first parameter, `this`, so
`h.count(b)` becomes something like `Histogram::count(&h, b)`. On x86-64
it arrives in `rdi`, like any first argument. `const` on the function
makes `this` a `const Histogram*`.

**struct vs class.** They're the same thing. The only difference is the
default: struct members are public unless you say otherwise, class
members are private. The convention is to use struct for plain data and
class when there's an invariant to protect. The `File`-like class in lesson
1.3.4 is a class.

**2 KiB of counters.** 256 × 8 bytes is 2 KiB, which fits easily in a
CPU's 32–48 KiB L1 data cache, so `add` runs at about a byte per cycle.
`std::array` is a C array with `.size()` and no decay to a pointer, and
adds no overhead.

**Byte histograms are a real tool.** Compressed or encrypted data has a
nearly flat histogram, text is lumpy around the letters, and x86 machine
code has spikes at common opcodes. `binwalk -E` plots the entropy of a file
from this kind of count.
