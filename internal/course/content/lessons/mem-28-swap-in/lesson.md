---
title: "wordfreq: your containers at work"
minutes: 40
files: [wordfreq.cpp]
run:
  args: ["-n", "5", "poem.txt"]
  files:
    poem.txt: "The fog comes\non little cat feet.\n\nIt sits looking\nover harbor and city\non silent haunches\nand then moves on.\n"
deeper:
  - title: "Programming Pearls: a literate program (Bentley, Knuth, McIlroy)"
    url: https://dl.acm.org/doi/10.1145/5948.315654
---
In 1986, Jon Bentley asked Donald Knuth to write a program that prints
the most common words in a text. Knuth wrote ten pages of careful
Pascal with a custom hash trie. Doug McIlroy replied with six lines of
shell:

```
tr -cs A-Za-z '\n' | tr A-Z a-z | sort | uniq -c | sort -rn | head
```

Now you write it in C++ with **your own containers**: `String` for the
words, `HashMap<String, std::size_t>` to count them, and
`Vector<WordCount>` to sort and trim. No `std::string`, `std::vector`
or `std::unordered_map` for the data. The function signatures in
`wordfreq.hpp` take your types.

`string.hpp`, `vector.hpp` and `hashmap.hpp` here are the course's
versions of what you built in this chapter, so a leftover bug in yours
can't follow you into this lesson.

The output has to match McIlroy's pipeline (with `LC_ALL=C`), so the
details matter:

- a word is a run of ASCII letters, lowercased. Digits, `'`, `-` and
  bytes above 127 all separate words
- the highest count comes first. Equal counts go in byte order, so
  `apple` comes before `banana`
- each line is the count right-aligned in 7 columns, a space, then the
  word, which is `uniq -c`'s format: `std::format("{:7} {}\n", ...)`

```
$ ./wordfreq -n 5 poem.txt
      3 on
      2 and
      1 cat
      1 city
      1 comes
```

## Assignment

- [ ] `count_words` splits the input into lowercase words and counts them in the `HashMap`
- [ ] `top_words` collects the counts with `for_each`, sorts them, and keeps the first `n`
- [ ] `main` takes `-n N` and any number of files (standard input if none), and prints in `uniq -c` format
- [ ] A file that can't be opened is reported as `wordfreq: FILE: can't open`, the rest are still counted, and the exit status is 1. A bad `-n` is a usage error with status 2
