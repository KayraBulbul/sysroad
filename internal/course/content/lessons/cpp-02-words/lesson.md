---
title: Counting words
minutes: 15
files: [words.cpp]
deeper:
  - title: std::basic_string_view (cppreference)
    url: https://en.cppreference.com/w/cpp/string/basic_string_view
  - title: std::isspace (cppreference)
    url: https://en.cppreference.com/w/cpp/string/byte/isspace
---
`wc -w` counts words. Before reading files, get the counting right on a
piece of text.

**`std::string_view`** is a pointer and a length that *looks at* someone
else's characters. Nothing is copied and it owns nothing, a lot like a Go
string. A `std::string`, a string literal or a slice of a bigger buffer all
convert to one for free:

```cpp
std::size_t count_words(std::string_view text);

count_words("two words");                    // literal
count_words(line);                           // a std::string
count_words(std::string_view(buf, n));       // part of a buffer
```

The one rule: a view must not outlive the characters it points at.

**Classifying bytes.** `std::isspace` from `<cctype>` takes an `int`. On
most machines `char` is signed, so UTF-8 bytes like `é` are negative. Passing
a negative value is undefined behaviour, so always cast first:

```cpp
if (std::isspace(static_cast<unsigned char>(c))) { ... }
```

**The spec.** A word is a run of one or more non-whitespace bytes.
Whitespace means `' '`, `'\t'`, `'\n'`, `'\v'`, `'\f'` and `'\r'`.
Everything else, punctuation and UTF-8 bytes included, is part of a word.

```text
"hello world"        2
"  padded  "         1
"a,b c"              2
""  and  "   "       0
```

## Assignment

- [ ] Implement `count_words` in one pass over the text
- [ ] Don't copy the text into a `std::string`
