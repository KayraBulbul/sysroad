On Linux the errors read `multiple definition of 'mean(…)'` and
`undefined reference to 'report[abi:cxx11](…)'`. On the Mac, `duplicate
symbol '_verbosity'` and `Undefined symbols for architecture arm64`.
The `[abi:cxx11]` is a tag that libstdc++ adds to functions returning
`std::string`; it's part of the mangled name.
---
In `stats.hpp`:

```cpp
extern int verbosity;                         // a declaration: no storage
inline double mean(const std::vector<double>& xs) { … }
```

and in `stats.cpp`, outside any function: `int verbosity = 1;`
---
In both `.cpp` files, wrap `less_than` in an anonymous namespace:

```cpp
namespace {
bool less_than(double a, double b) { … }
}  // namespace
```

and in `report.cpp`, add the missing `const` to `report`'s parameter.
