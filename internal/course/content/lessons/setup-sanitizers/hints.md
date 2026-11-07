2: `push_back` on a full vector allocates a new buffer, moves the
elements, and frees the old one. `first` is a reference into the *old*
one. Copy the value out before growing.
---
3: `INT_MAX + INT_MAX` doesn't fit in an `int`. Do the addition in
`long long`, which has room for any two ints, then divide.
---
4: `greeting` builds `s`, returns a view of it, and `s` is destroyed at
the `}`. Make `greeting` return a `std::string`.
---
5: `c - '0'` is only 0–9 for digit characters. `'-'` gives -3, which
writes *before* the array. Skip anything that isn't `'0'`–`'9'`.
