---
title: "Preprocess, compile, assemble, link"
minutes: 30
files: [macros.hpp]
program: stages
run: {}
deeper:
  - title: "CS:APP 7.1-7.2: compiler drivers and static linking"
    url: https://csapp.cs.cmu.edu/
  - title: "The C preprocessor: stringizing, and macros that expand twice (GCC docs)"
    url: https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
  - title: "Predefined macros (cppreference)"
    url: https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros
---
`clang++ main.cpp util.cpp -o prog` looks like one step. It's four,
and you can stop after any of them:

```sh
clang++ -E main.cpp -o main.ii   # 1. preprocess: C++ source, with #includes pasted in
clang++ -S main.ii  -o main.s    # 2. compile: assembly, as in phase 4
clang++ -c main.s   -o main.o    # 3. assemble: an object file of machine code
clang++ main.o util.o -o prog    # 4. link: object files and libraries into a program
```

Each `.cpp` file goes through the first three stages on its own, as a
**translation unit**. Only the linker sees all of them together. So
every error message belongs to a stage, and knowing which tells you
where to look: `error: unknown type name` is the compiler,
`undefined reference to` (or `Undefined symbols`, on the Mac) is the
linker.

The rest of this chapter is about stage 4. This lesson is about stage
1, the **preprocessor**. It runs before the compiler and knows nothing
about C++: it works on tokens of text. `#include` pastes a whole file
in (`clang++ -E` on a three-line file that includes `<iostream>` is
tens of thousands of lines). `#define` makes a macro, which is
replaced by its body wherever it appears, and `#if` keeps or drops
lines.

Because it's text, macros have traps of their own:

- **Precedence.** `#define SQUARE(x) x * x` makes `SQUARE(1 + 2)` into
  `1 + 2 * 1 + 2`, which is 5. Parenthesise every use of a parameter,
  and the whole body.
- **Stringizing.** `#x` in a macro body turns the argument into a
  string literal, which is how `assert` prints the condition that
  failed. But `#` uses the argument as written: `#define STR(x) #x`
  makes `STR(__LINE__)` into `"__LINE__"`. Arguments are only
  expanded when they're passed on to another macro, so the fix is two
  levels: `STR(x)` calls `STR_NO_EXPAND(x)`, which does the `#`.
- **Gluing literals.** String literals next to each other are joined
  by the compiler: `"a" "b"` is `"ab"`. That's how to build `"file:line"`
  out of `__FILE__`, `":"` and a stringized `__LINE__`, at compile
  time.

The compiler also **predefines** macros that describe the target:
`__x86_64__` or `__aarch64__`, `__linux__` or `__APPLE__`, and
hundreds more. `clang++ -dM -E -x c++ /dev/null` lists them all. `#if
defined(__aarch64__)` is how portable code picks a code path per CPU
or OS without anything happening at run time.

Today, most macros are better written as `constexpr` values, `inline`
functions and templates, which obey scopes and types. What only
macros can do is look at the *text* of your code, or at where it is.

## Assignment

- [ ] Fix `SQUARE`, and write `COUNT_OF` with `sizeof`
- [ ] `STR` expands its argument before stringizing it, and `HERE` is a literal `"file:line"`
- [ ] `SHOW(expr)` gives `"expr = value"` using `show()`, and evaluates `expr` once
- [ ] `arch()` and `os()` use `#if defined(…)` on the predefined macros
- [ ] Try the four commands above on `main.cpp`, and look at what each one makes
