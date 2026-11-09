---
title: "The rules of zero, three and five"
minutes: 20
files: [matrix.hpp, matrix.cpp]
run: {}
deeper:
  - title: The rule of three/five/zero (cppreference)
    url: https://en.cppreference.com/w/cpp/language/rule_of_three
  - title: "Rule of Zero (R. Martinho Fernandes)"
    url: https://web.archive.org/web/20130211035910/http://flamingdangerzone.com/cxx11/2012/08/15/rule-of-zero.html
---
Five special member functions decide what happens when an object is
destroyed, copied or moved: the destructor, copy constructor, copy
assignment, move constructor and move assignment. They come as a set.

**The rule of three.** If a class needs a custom destructor, it almost
certainly owns something by hand, and the generated copy operations
will copy the handle, not the thing. So it needs custom copies too.
That was `Buffer` two lessons ago.

**The rule of five.** Declaring a destructor or a copy operation also
*switches off* the compiler-generated moves, so every "move" quietly
becomes a copy. A class that manages a resource should write all five,
or `= delete` the ones it can't support.

**The rule of zero.** Better still: don't manage the resource by hand.
Hold it in a member that already follows the rules, like
`std::vector`, `std::string`, `std::unique_ptr` or your `Fd`, and write
none of the five. The compiler-generated versions then do the right
thing member by member.

`Matrix` breaks the rule of three. It has a destructor that frees its
`double*`, and nothing else. Copy one and you get two matrices sharing
one buffer, then a double free. `main.cpp` does exactly that.

Fix it either way. Writing all five is good practice, and switching to
a `std::vector<double>` is the rule of zero. Either way, the tests check
that copies are deep and that moves don't allocate.

## Assignment

- [ ] Copying a `Matrix` makes an independent copy
- [ ] Copy assignment works, including `m = m`
- [ ] Moving a `Matrix` doesn't allocate
- [ ] Nothing leaks or is freed twice
