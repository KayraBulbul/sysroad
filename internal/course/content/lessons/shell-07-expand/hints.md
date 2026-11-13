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
`expand_word` walks the word again, building `out`, with a flag `kept`
that turns true for any quote, backslash or plain character. At the
end, `!kept && out.empty()` means the word vanishes.
---
Put the `$` handling in one helper that both the unquoted and the
double-quoted parts call:

```cpp
// Expands the $ at word[i] into out; returns the index after it.
static size_t expand_dollar(const std::string& word, size_t i,
                            const Shell& sh, std::string& out);
```

`lookup_var(sh, name).value_or("")` gives a variable's value.
