**Why the scale is only 1, 2, 4 or 8.** Those are the sizes of the
basic types, and the scale is encoded in two bits of the instruction.
For a 24-byte `Item` the compiler can't write `(%rdi,%rcx,24)`. It
either multiplies the index itself or, as in `november`, gives up on
the index and walks a pointer forward 24 bytes at a time. Turning
`items[i]` into a moving pointer is called *strength reduction*.

**Why `november` starts at offset 16.** The compiler is free to keep
any pointer it likes, as long as the loads come out right. Pointing
at the `count` field and reaching back 8 bytes for `price` is as good
as pointing at the start. Decompilers see this constantly, and it's
one reason their output looks odd.

**Struct layout is part of the ABI.** The offsets are baked into
every piece of code that touches an `Item`. If a library is compiled
with one field order and your program with another, both compile,
they link, and they read each other's fields wrong. That's why
changing a struct in a shared library's header is a breaking change,
and why the tests here can catch a wrong order at all.

**Bounds don't exist.** `movl 8(%rdi,%rsi,4), %eax` loads whatever is
at that address. Nothing in the instruction knows how long the array
is. Every out-of-bounds bug from phase 2 is this: a correct load from
an address that belongs to something else. ASan works by adding a
check before each load and store.

**Cache behaviour is visible here.** `kilo` touches memory in order,
8 bytes at a time, which the CPU's prefetcher predicts perfectly.
`papa` can't load the next node until the current one has arrived,
and each node may be anywhere on the heap. That difference is most of
phase 5.

**C++ references are pointers.** A `const Item&` parameter compiles
to exactly the same code as a `const Item*`: an address in `rdi`. So
do `this` in a member function, which is a hidden first argument, and
`std::span`, which is a pointer and a length in two registers.
