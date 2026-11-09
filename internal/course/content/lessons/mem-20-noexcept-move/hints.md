For `Record`, add `noexcept` to the move constructor and the move
assignment operator, after the parameter list:
`Record(Record&& o) noexcept : ...` and
`Record& operator=(Record&& o) noexcept { ... }`.
---
For `Line`, ask for the compiler's moves back:

```cpp
Line(Line&&) noexcept = default;
Line& operator=(Line&&) noexcept = default;
```

Defaulted moves are `noexcept` whenever every member's move is, as
`std::string`'s is. Or delete the hand-written copy constructor, and
the copy assignment too: with none of the five declared, the compiler
generates all of them (the rule of zero).
