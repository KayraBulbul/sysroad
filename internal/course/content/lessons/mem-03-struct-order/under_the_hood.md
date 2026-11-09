**Why compilers don't do this for you.** The order is part of the
contract: C code, file formats and other compilers rely on the first
field being at offset 0 and the rest in order. Rust *does* reorder
fields by default, and you opt out with `#[repr(C)]`.

**Padding isn't zeroed.** The gap bytes hold whatever was there before.
`memcmp` on two equal structs can say they differ, and writing a struct
straight to a file or socket can leak old memory. That's a real class of
kernel bug: an infoleak through padding. Compare field by field, or
`memset` to zero first.

**`[[no_unique_address]]`** (C++20) lets an empty member, like a
stateless allocator or comparator, take no space at all.

**Hot and cold splitting.** Sometimes the better move is splitting the
struct: fields used in the inner loop in one array, rarely used ones in
another. That's the idea behind *data-oriented design*, and you'll
measure it in phase 5.
