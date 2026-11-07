Keep one `bool in_word = false;`. A new word starts each time you go from
whitespace to non-whitespace, so that's where to count.
---
```cpp
for (char c : text) {
    bool space = std::isspace(static_cast<unsigned char>(c));
    if (!space && !in_word) { ++count; }
    in_word = !space;
}
```
