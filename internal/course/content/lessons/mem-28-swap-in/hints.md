Read a char at a time and keep the word so far:

```cpp
String word;
char c;
while (in.get(c)) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        word.push_back(static_cast<char>(c | 0x20));   // ASCII lowercase
    } else if (!word.empty()) {
        ++counts[word];
        word.clear();
    }
}
```

Don't forget the last word, if the input doesn't end with a separator.
---
`top_words`: copy each entry into a `Vector`, then sort with a lambda:

```cpp
counts.for_each([&](const String& w, std::size_t n) { all.push_back(WordCount{w, n}); });
std::sort(all.begin(), all.end(), [](const WordCount& a, const WordCount& b) {
    return a.count != b.count ? a.count > b.count : a.word < b.word;
});
```

Then `all.erase(all.begin() + n, all.end())` if there are more than `n`.
---
For `-n`, `std::from_chars` into a `std::size_t` rejects negatives and
junk: check `ec == std::errc()` and that it consumed the whole
argument. Open files with `std::ifstream in(argv[i], std::ios::binary);`.
`std::cin` is a `std::istream` too, so `count_words(std::cin, counts)`
handles the no-files case.
