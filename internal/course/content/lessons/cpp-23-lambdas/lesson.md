---
title: Lambdas and <algorithm>
minutes: 20
files: [files.cpp]
run:
  args: ["."]
  files:
    notes.txt: "short\n"
    report.pdf: "pretend this is a much longer pdf file\n"
    empty.log: ""
    data.csv: "a,b,c\n1,2,3\n"
deeper:
  - title: Lambda expressions (cppreference)
    url: https://en.cppreference.com/w/cpp/language/lambda
  - title: Algorithms library (cppreference)
    url: https://en.cppreference.com/w/cpp/algorithm
---
`<algorithm>` has the loops you'd otherwise write by hand: sort, find,
count, remove, max. You tell each one *what* to do with a **lambda**, a
function written inline:

```cpp
std::sort(v.begin(), v.end(), [](const Entry& a, const Entry& b) {
    return a.size > b.size;   // "a comes before b"
});
```

The `[]` is the **capture list**: which local variables the lambda can
use.

```cpp
std::uint64_t min = 1024;
auto big = [min](const Entry& e) { return e.size >= min; };  // copies min
auto n = std::count_if(v.begin(), v.end(), big);
```

`[min]` copies, `[&min]` refers to the original, `[&]` captures everything
used by reference, and `[=]` by copy. A lambda is an object, so you can
store it in an `auto` variable and pass it around.

The ones you need here are `std::sort`, `std::accumulate` (from
`<numeric>`), `std::count_if`, `std::max_element`, and `std::erase_if`,
C++20's one-liner for "remove everything matching".

This lesson's program is a tiny "what's taking up space": it lists a
folder's files, biggest first.

**The spec**

- `sort_by_size`: biggest first. Equal sizes go in name order.
- `total_size`: the sum of every size.
- `count_at_least`: how many entries have `size >= min`.
- `largest`: the biggest entry (the first in the list if several tie), or
  `nullopt` for an empty list.
- `remove_empty`: removes every entry of size 0.
- `names_with_ext`: the names that end in `ext` (like `".txt"`), in list
  order.

`main` is done. It reads the folder, calls your functions, and prints the
result.

## Assignment

- [ ] `sort_by_size` with `std::sort` and a lambda
- [ ] `total_size` with `std::accumulate`, `count_at_least` with `std::count_if`
- [ ] `largest` with `std::max_element`, `remove_empty` with `std::erase_if`
- [ ] `names_with_ext`, with a loop or an algorithm
