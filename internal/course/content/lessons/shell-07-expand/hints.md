In the lexer's word loop, a quote skips ahead to its closing quote:

```cpp
if (c == '\'') {
    size_t close = line.find('\'', i + 1);
    if (close == std::string::npos) throw SyntaxError("unterminated '");
    i = close + 1;
} else if (c == '"') {
    ++i;
    while (i < n && line[i] != '"') {
        i += line[i] == '\\' ? 2 : 1;   // \" doesn't close it
    }
    if (i >= n) throw SyntaxError("unterminated \"");
    ++i;
} else if (c == '\\') {
    i = std::min(i + 2, n);
} else {
    ++i;
}
```

and the word is `line.substr(start, i - start)`.
---
Expanding walks the word again, building `out`, with a flag `kept`
that turns true for any quote, backslash or plain character. At the
end, `!kept && out.empty()` means the word vanishes: return
`std::optional<std::string>`, with `std::nullopt` for that.
---
Put the `$` handling in one helper that both the unquoted and the
double-quoted parts call:

```cpp
// Expands the $ at word[i] into out; returns the index after it.
static size_t expand_dollar(const std::string& word, size_t i,
                            const Shell& sh, std::string& out);
```

A function that looks a variable up, giving `std::optional<std::string>`,
is worth having now: next lesson it looks in the shell's own variables
first. For now it's `getenv`, with `.value_or("")` for the unset case.
---
Expanding at run time: just before a pipeline runs, make a copy of it
with every word expanded (dropping the ones that vanish) and every
redirection's target expanded, and run the copy. Keep the words as
written in the job's text.
