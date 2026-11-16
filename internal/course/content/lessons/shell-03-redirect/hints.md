A token is a kind (word or operator), its text, and the fd number
(-1 for none). The lexer's loop: skip spaces and tabs; stop at a `#`;
try to read an operator; otherwise read a word up to a space, tab, `<`
or `>`. Reading an operator starts by skipping digits: if what follows
is `<` or `>`, the digits were its fd number; if not, they belong to a
word, and you start again at the first digit as a word.
---
After the `<` or `>`, the operator is two characters if the next one
is `&`, or for `>` another `>`. `std::stoi` turns the digits into the
fd, once you've checked there are at most 9 of them.
---
A redirection is what to do (read, write, append, copy), the fd, and
either a path or the fd to copy. Parsing walks the tokens: a word goes
on the command's words; an operator takes the next token, which must be
a word, and makes a redirection. A `std::runtime_error` subclass for
syntax errors, caught in `run_line`, gives the message and the 2.
---
Applying one redirection:

```cpp
int flags = O_RDONLY;                                    // <
if (r.op == Write) flags = O_WRONLY | O_CREAT | O_TRUNC;  // >
if (r.op == Append) flags = O_WRONLY | O_CREAT | O_APPEND; // >>
int fd = open(r.path.c_str(), flags, 0666);
// fd == -1: throw, naming r.path
if (fd != r.fd) { dup2(fd, r.fd); close(fd); }
```

For a copy, check the source is open first: `fcntl(from, F_GETFD)` is
-1 if it isn't. `std::system_error(errno, std::generic_category(), path)`
makes an exception whose `what()` is `path: reason`.
