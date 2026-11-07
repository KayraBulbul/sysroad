---
title: Exceptions, and when not to use them
minutes: 20
files: [config.cpp]
run:
  args: ["app.conf"]
  files:
    app.conf: "# server settings\nport = 8080\nhost = localhost\n\nthis line is wrong\n"
deeper:
  - title: Exceptions (cppreference)
    url: https://en.cppreference.com/w/cpp/language/exceptions
  - title: "C++ Core Guidelines: E.2 Throw an exception to signal that a function can't perform its assigned task"
    url: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Re-throw
---
Last lesson's rule was that if failing is a normal answer, return it
(`std::optional`). Exceptions are for the other kind of failure: the
function *can't do its job*, the caller may be several frames up, and
checking at every level in between would bury the real code.

A config file parser is a good example. A bad line on line 40 should stop
everything and say where:

```text
$ ./config app.conf
config: line 5: expected key = value
```

```cpp
throw ConfigError(line_no, "expected key = value");  // leaves the function now

try {
    auto cfg = parse_config(file);
    print(cfg);
} catch (const ConfigError& e) {  // catch by const reference
    std::cerr << "config: " << e.what() << '\n';
    return 1;
}
```

`throw` unwinds the stack. Every function between the `throw` and the
`catch` returns at once, **running the destructors** of its locals on the
way out. That's why RAII and exceptions go together: nothing leaks, even
mid-parse.

`ConfigError` derives from `std::runtime_error`, which stores the message
and returns it from `what()`. You'll write its constructor, which passes
a combined message to the base class:

```cpp
ConfigError::ConfigError(int line, const std::string& msg)
    : std::runtime_error("line " + std::to_string(line) + ": " + msg), line_(line) {}
```

**The format**

- One `key = value` per line. Spaces around the key and the value don't
  count. The value may contain spaces and even `=`.
- Lines that are blank, or whose first non-space character is `#`, are
  skipped.
- Errors, with 1-based line numbers: a line with no `=` gives `expected
  key = value`, an empty key gives `empty key`, and a key seen before gives
  `duplicate key 'port'`.
- `main` prints `key=value` for each entry in key order, or the error
  with exit status 1.

## Assignment

- [ ] Write the `ConfigError` constructor
- [ ] Implement `parse_config`, throwing `ConfigError` for each bad line
- [ ] Catch it in `main`, print `config: ...` and return 1
