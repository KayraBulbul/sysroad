A parameter taken by value is your own copy, so you're free to move
from it: `out.push_back(std::move(item));`. The caller decides whether
the parameter was filled by a copy or a move, and the tests pass
rvalues.
---
`Tracked t = std::move(v.back()); v.pop_back(); return t;`. Returning
a local moves (or elides the move entirely), so don't write
`return std::move(t);`. That actually blocks the elision.
---
`concat`: iterate `b` by non-const reference and move each element:
`for (Tracked& t : b) a.push_back(std::move(t));`. Then `return a;`.
A by-value parameter is moved when returned.
---
`rotate3`: `Tracked tmp = std::move(a); a = std::move(b); b = std::move(c); c = std::move(tmp);`.
For `longest`, `for (const Tracked& t : v)` looks at each element
without copying it.
