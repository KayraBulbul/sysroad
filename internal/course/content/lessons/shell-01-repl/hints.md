A layout that grows well: `conch.hpp` (`Shell`, `run_line` and your own
declarations), `main.cpp` for the loop, and a file each for splitting
and running. `main` is short:

```cpp
int main() {
    Shell sh;
    bool interactive = isatty(0);
    for (;;) {
        if (interactive) std::cout << "conch$ " << std::flush;
        std::optional<std::string> line = read_line(0);   // yours
        if (!line) break;
        run_line(sh, *line);
    }
    return sh.last_status;
}
```
---
Reading a line: `read(fd, &c, 1)` in a loop. 1 means a byte: a `\n`
ends the line, anything else goes on the end. 0 means the end of the
input: return the line if it has anything in it, else "no line"
(`std::nullopt`). -1 with `errno == EINTR` means try again.
---
Splitting: walk the line one character at a time with a `word` string
and an `in_word` flag. A space or tab ends the current word (if there
is one); a `#` when you're not in a word ends the whole loop; anything
else goes on the end of the word. Don't forget the last word.
---
Running, with the argument array `execvp` wants:

```cpp
std::vector<char*> args;
for (const auto& a : words) args.push_back(const_cast<char*>(a.c_str()));
args.push_back(nullptr);
std::fflush(nullptr);
pid_t pid = fork();
if (pid == 0) {
    execvp(args[0], args.data());
    // only reached if it failed: ENOENT means not found (127), else 126
}
int wstatus = 0;
waitpid(pid, &wstatus, 0);   // again on EINTR
```

Then `WIFSIGNALED(wstatus) ? 128 + WTERMSIG(wstatus) : WEXITSTATUS(wstatus)`.
