**Why decay exists.** C was designed so that passing an array never
copied it. On a 1970s PDP-11, copying a big array by accident would
have been expensive. C++ kept the rule for compatibility, and added
`std::array` and `std::span` (next lesson) for when you want the length
to come along.

**Where the length can survive.** A template can take an array *by
reference*, and then the length is part of the type:
`template <typename T, std::size_t N> std::size_t length(const T (&)[N]) { return N; }`.
`std::size(scores)` and range-for over a local array both use this
trick. It doesn't work once the array has already decayed to a pointer.

**`std::string` and `char[]`.** A string literal `"hi"` is a
`const char[3]`: two letters plus the `\0` that C uses as its end
marker. It decays to `const char*`, which is why `main` gets
`char** argv`. C strings measure their length by walking to the `\0`
(`strlen`), so the length is never stored.

**The warning you'd normally get.** clang warns about `sizeof` on an
array parameter (`-Wsizeof-array-argument`). `main.cpp` switches the
warning off on purpose, to show you the mistake running.
