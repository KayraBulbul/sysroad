The parent's side is short:

```cpp
std::fflush(nullptr);
pid_t pid = fork();
if (pid == -1) {
    throw std::system_error(errno, std::generic_category(), "fork");
}
if (pid > 0) {
    return pid;
}
// only the child gets here
```
---
In the child, nothing may leave the function: not a return, not an
exception. Catch everything and fall through to `_exit`:

```cpp
int code = 1;
try {
    code = body();
} catch (...) {
}
std::fflush(nullptr);
_exit(code);
```
---
Two loops: the first spawns all the children and keeps their PIDs,
the second waits for each PID in turn. The lambda for child `i`
captures `i` by value: `spawn([&body, i] { return body(i); })`.
