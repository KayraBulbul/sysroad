Count the columns first:
`int shown = o.lines + o.words + o.bytes;` (each `bool` adds 0 or 1).
The number of inputs is `std::max<std::size_t>(o.files.size(), 1)`, since
no files means stdin. If both are 1, return 1.
---
Start with `int min_width = o.files.empty() ? 7 : 1;` and
`std::uintmax_t total = 0;`. For each name, `"-"` sets `min_width = 7`.
Otherwise look at the path: regular files add their size, and things
that exist (`fs::exists(p, ec)`) but aren't regular set `min_width = 7`.
---
Digits of `total`: start at `int width = 1;` and
`while (total >= 10) { total /= 10; ++width; }`. Return
`std::max(width, min_width)`.
