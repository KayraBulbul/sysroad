**Iterator categories.** Algorithms ask for what they need. `std::find`
only needs to step forward (an *input iterator*). `std::reverse` needs
to step back (*bidirectional*), and `std::sort` needs `it + n` and
`it2 - it1` (*random access*). A pointer is the strongest kind there
is, *contiguous*, which is why a `T*` works everywhere. A linked list's
iterator is only bidirectional, so `std::sort` won't compile on a
`std::list` (it has its own `list::sort`).

**`std::erase_if` is in the standard now.** C++20 added
`std::erase_if(v, pred)` for every standard container, written the
way you just wrote yours. Before that, the erase–remove idiom was a
well-known trap: `v.erase(std::remove_if(...))` without the second
argument erases only *one* element.

**Debug iterators.** libstdc++'s `-D_GLIBCXX_DEBUG` and MSVC's debug
mode make iterators fat. Each one knows its container, and they check
invalidation on every use, so the "use after erase" bug fails with a
clear message instead of silently reading the wrong element. It's slow,
but it's very good at finding these bugs.

**Why `insert` is O(n).** Every element after the position moves. If you
insert at the front a lot, a `std::deque` or building in reverse and
then calling `std::reverse` is far faster. Phase 5 has you measure it.
