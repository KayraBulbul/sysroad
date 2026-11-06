---
title: echo
minutes: 15
files: [echo.cpp]
run:
  args: [hello, from, C++]
deeper:
  - title: std::basic_string (cppreference)
    url: https://en.cppreference.com/w/cpp/string/basic_string
  - title: std::vector (cppreference)
    url: https://en.cppreference.com/w/cpp/container/vector
---
Your first coreutil. `echo` prints its arguments with one space between
each, then a newline.

```text
$ ./echo hello   world
hello world
$ ./echo -n hi
hi$
```

`main` still gets C strings, but you only touch them once, to copy them
into real strings:

```cpp
int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);  // skip argv[0]
}
```

`std::string` owns its characters, knows its length and grows with `+=`.
`std::vector<T>` is a growable array, like a Go slice that owns its memory.

Pass big things by **const reference**. That's no copy, and the function
can't change them:

```cpp
std::string join_args(const std::vector<std::string>& args) {
    for (const auto& a : args) { /* each argument, not copied */ }
}
```

Print with `std::cout << text << '\n';`.

**The spec**

- Join the arguments with single spaces and print them, then `'\n'`.
- If the *first* argument is exactly `-n`, don't print it and leave off
  the newline. `echo hi -n` prints `hi -n`.
- With no arguments, print just `'\n'`.
- Print every argument exactly as given, inner spaces included.
- GNU echo also has `-e`, `-E` and combined flags like `-ne`. They aren't
  part of this lesson.

## Assignment

- [ ] Implement `join_args`
- [ ] In `main`, copy the arguments into a vector, handle a leading `-n`, and print
