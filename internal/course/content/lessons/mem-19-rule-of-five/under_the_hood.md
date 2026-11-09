**Which members does the compiler generate?** Roughly: declare *any*
of the destructor, copy constructor or copy assignment, and the move
operations aren't generated, so moves fall back to copies. Declare a
move operation, and the copies are deleted. The full table is on the
cppreference page, and it's worth looking at once. "Write all or
nothing" means you never have to remember it.

**`= default` is a declaration.** `~Matrix() = default;` still counts
as declaring a destructor, so it still switches off the implicit moves.
If you need to write it (say, to make a destructor `virtual`), default
the other four too.

**Rule of zero scales.** A class built only from rule-following
members follows the rules too, so the property composes. `Image` holds a
`Matrix`, `Document` holds a vector of `Image`s, and none of them write
a single special member. This is the main reason modern C++ code has so
few `delete`s in it.

**Where the rule of five belongs.** The classes that implement the
members everyone else uses: `std::vector`, `std::string`,
`std::unique_ptr`, your `Fd`, and the `String` and `Vector` you'll
build next chapter. They're small, carefully tested, and everything
else is built on them.
