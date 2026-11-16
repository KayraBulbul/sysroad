Each builtin is a function with the same shape, so they fit in a table:

```cpp
using Builtin = int (*)(Shell&, const std::vector<std::string>&);

Builtin find_builtin(const std::string& name) {
    static const std::map<std::string, Builtin> builtins = {
        {"cd", builtin_cd}, {"pwd", builtin_pwd}, /* ... */
    };
    auto it = builtins.find(name);
    return it == builtins.end() ? nullptr : it->second;
}
```

In `run_line`, `if (Builtin b = find_builtin(words[0]))` runs it and
skips the fork.
---
`cd`: work out the folder (`$HOME` with no argument, `$OLDPWD` for
`-`), remember `getcwd` from before, `chdir`, and only if that worked,
`setenv` `OLDPWD` to the old folder and `PWD` to the new `getcwd`.
`std::strerror(errno)` gives the reason, for the message.
---
`exit`: `std::strtol(s.c_str(), &end, 10)`, and the number is good if
`s` isn't empty and `*end == '\0'`. The code is `n & 0xff`. Set
`exiting` and the code in your `Shell`; in `main`, after `run_line`,
`if (sh.exiting) return sh.exit_code;`.
---
`type` searches `PATH` the way `execvp` does, as in lesson 6.1.3:
split it at `:`, try `dir + "/" + name` for each, and take the first
that's a regular file you may execute
(`stat` with `S_ISREG`, and `access(path, X_OK) == 0`). A name with a
`/` isn't searched for: it's found only if it's such a file itself.
