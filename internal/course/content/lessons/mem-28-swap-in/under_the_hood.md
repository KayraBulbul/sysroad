**Who won?** McIlroy's review said Knuth's program was a masterpiece
of engineering for a problem that a few standard tools already solved.
Both points stand. The pipeline sorts every word, which is O(n log n)
and needs the whole text in temporary files. A hash table counts in
O(n) and only keeps the distinct words. On a big corpus yours is much
faster. Try it on a novel from Project Gutenberg, against the pipeline
with `time`.

**`std::partial_sort`.** You only need the top `n`, so sorting
everything is wasted work. `std::partial_sort(first, first + n, last, cmp)`
puts just the first `n` in order, in O(m log n) for `m` distinct words.
For a top 10 of 50,000 words, that's a real saving.

**Why the bytes, not the locale?** `sort` without `LC_ALL=C` uses your
locale's collation, which ignores case and punctuation and orders
accented letters by language rules, so its output differs between
machines. Byte order is boring and reproducible, and that's why scripts
pin `LC_ALL=C`.

**Unicode.** "Naïve" is two words here, because `ï` is two bytes above
127. Real word splitting follows Unicode's segmentation rules (UAX #29)
and needs a library like ICU. Lowercasing isn't one-to-one either: `ß`
uppercases to `SS`.
