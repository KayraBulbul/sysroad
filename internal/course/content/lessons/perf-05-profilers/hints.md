In `perf report`, `count_words` sits near the top with `find_word`
inlined into it, and much of the rest is `memcmp` (or `bcmp`), called
by `std::string`'s `==`. On the Mac, `count_words` and
`_platform_memcmp`. Every word is compared against every distinct word
seen so far: n words times d distinct words. With 150,000 words and
over 20,000 distinct, that's billions of string compares.
---
A hash table makes each lookup take about the same time however many
words there are:

```cpp
std::unordered_map<std::string, long> seen;
for (const std::string& w : words) {
    ++seen[w];
}
```

`operator[]` inserts a 0 for a new word, so `++` counts it either way.
Then copy the entries into a `std::vector<WordCount>`.
---
The other suspects? `word = word + c` builds a new string for every
letter, but words are short and the strings stay inside the object
(the small-string optimisation), so it costs little. Sorting 20,000
counts takes a millisecond or so. Changing them makes the code
nicer, but the profile says they were never the problem.
