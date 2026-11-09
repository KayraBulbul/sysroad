---
title: Reordering fields to shrink a struct
minutes: 10
files: [pack.cpp, record.hpp]
run:
  args: ["--pack", "char", "double", "char", "int"]
deeper:
  - title: The Lost Art of Structure Packing (Eric S. Raymond)
    url: http://www.catb.org/esr/structure-packing/
---
C++ never reorders your fields. They're laid out in the order you wrote
them, so the order decides how much padding you pay for:

```cpp
struct Loose { char a; double d; char b; int n; };  // 24 bytes
struct Tight { double d; int n; char a; char b; };  // 16 bytes
```

The recipe: **sort fields by alignment, largest first.** Each field then
starts right where the previous one ended, because a multiple of 8 is
also a multiple of 4, 2 and 1. Only tail padding is left.

Does 8 bytes matter? For one object, no. For a million of them in a
`std::vector`, that's 8 MB, and fewer objects per cache line means more
trips to memory when you loop over them. Hot data structures in games,
databases and kernels are laid out on purpose.

`layout.cpp` is your code from the last lesson. `main.cpp` now takes
`--pack` and shows the layout after your reordering:

```
$ ./pack --pack char double char int
as written: sizeof 24, 10 bytes of padding
packed:
offset  size  field
     0     8  double
     8     4  int
    12     1  char
    13     1  char
    14        (2 bytes padding)
sizeof 16, alignof 8, 2 bytes of padding
```

## Assignment

- [ ] `packed` reorders fields by alignment, largest first, keeping the original order among equals
- [ ] Reorder `Record` in `record.hpp` so `sizeof(Record)` drops from 40 to 24 (keep every field)
