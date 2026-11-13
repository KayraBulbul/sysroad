Walk the tokens with an index, since a redirection uses two:

```cpp
size_t i = 0;
while (i < tokens.size()) {
    if (tokens[i].kind == Token::Word) {
        cmd.argv.push_back(tokens[i].text);
        ++i;
    } else {
        // tokens[i] is the operator, tokens[i + 1] its target
        cmd.redirects.push_back(...);
        i += 2;
    }
}
```
---
```cpp
r.fd = op.fd != -1 ? op.fd : (op.text[0] == '<' ? 0 : 1);
```

For `<&` and `>&` the target must be all digits: `r.from =
std::stoi(target)`.
---
`apply_redirects` is your `apply_redirect` from 6.1.5 in a loop. For
`>&`, check the source fd is open with `fcntl(from, F_GETFD)` and throw
`std::system_error(EBADF, std::generic_category(), std::to_string(from))`
if not.
---
```cpp
SavedFds::SavedFds(const std::vector<Redirect>& redirects) {
    for (const Redirect& r : redirects) {
        // skip it if r.fd is already in saved_        int copy = fcntl(r.fd, F_DUPFD_CLOEXEC, 10);   // -1, EBADF if r.fd isn't open
        if (copy == -1 && errno != EBADF) {            copy = fcntl(r.fd, F_DUPFD_CLOEXEC, above);  // the limit is below 10:
                                                        // above every fd named
        }
        // still -1 and not EBADF: close the copies so far and throw
        saved_.emplace_back(r.fd, copy);
    }
}
```

The destructor goes through `saved_` backwards: `dup2(copy, fd)` and
`close(copy)`, or `close(fd)` if the copy is -1.
