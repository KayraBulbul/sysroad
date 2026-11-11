`SQUARE(x)` is `((x) * (x))`: brackets round each `x`, and round the
whole thing, so `100 / SQUARE(5)` is `100 / 25`.

`COUNT_OF(arr)` divides the array's size by one element's size:
`(sizeof(arr) / sizeof((arr)[0]))`.
---
Two macros make `STR`:

```cpp
#define STR_NO_EXPAND(x) #x
#define STR(x) STR_NO_EXPAND(x)
```

`STR(__LINE__)` passes `__LINE__` on to `STR_NO_EXPAND`, and arguments
are expanded on the way. Then `HERE` is three literals in a row:
`__FILE__ ":" STR(__LINE__)`.
---
`SHOW(expr)` is `show(#expr, (expr))`. `#expr` is the text, and
`(expr)` appears once, so it runs once.

For `arch()`:

```cpp
#if defined(__x86_64__)
    return "x86-64";
#elif defined(__aarch64__)
    return "arm64";
#else
    return "unknown";
#endif
```
