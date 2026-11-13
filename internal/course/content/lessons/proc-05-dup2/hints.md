Find the operator with `word.find_first_of("<>")`. Whatever's before
it must be empty or all digits (the fd); then check for `>>`, `>&`
and `<&` before the one-character `<` and `>`. Whatever follows is the
path, which mustn't be empty, or for `>&` and `<&`, a number.
---
The flags for each:

```cpp
O_RDONLY                              // <
O_WRONLY | O_CREAT | O_TRUNC          // >
O_WRONLY | O_CREAT | O_APPEND         // >>
```

The third argument to `open`, `0666`, is the new file's permissions
before the `umask` takes some away (usually leaving `0644`).
---
`open` returns the lowest free fd, which might already be the one you
want (`3<file` when 3 is free). Only `dup2` and `close` if it isn't:
closing it would undo the redirection.

For `>&m`, `dup2(m, n)` fails with `EBADF` if `m` isn't open. But if
`m == n`, `dup2` does nothing and succeeds even for a closed fd, so
check that one with `fcntl(n, F_GETFD) != -1`.
---
```cpp
if (pid == 0) {
    try {
        for (const auto& r : redirects) {
            apply_redirect(r);
        }
    } catch (const std::system_error& e) {
        std::fprintf(stderr, "redir: %s\n", e.what());
        _exit(1);
    }
    execvp(args[0], args.data());
    // ...
}
```
