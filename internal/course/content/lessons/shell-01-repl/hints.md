Walk the line one character at a time with a `word` string and an
`in_word` flag. A space or tab ends the current word (if there is
one); a `#` when you're not in a word ends the whole loop; anything
else goes on the end of the word. Don't forget the last word.
---
`run_command` is `start` and `wait_for` from lesson 6.1.4, with conch's
own message:

```cpp
if (pid == 0) {
    signal(SIGPIPE, SIG_DFL);
    execvp(args[0], args.data());
    if (errno == ENOENT) {
        std::cerr << "conch: " << argv[0] << ": command not found\n";
        _exit(127);
    }
    std::cerr << "conch: " << argv[0] << ": " << std::strerror(errno) << '\n';
    _exit(126);
}
return shell_status(wait_for(pid));
```

`std::fflush(nullptr)` before the fork, as always.
