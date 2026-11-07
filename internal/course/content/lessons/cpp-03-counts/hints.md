Declare `bool in_word = false;` *outside* the read loop, so its value
survives from one chunk to the next.
---
Inside the loop, `c.bytes += chunk.size();`, and for each `char ch` in the
chunk, `if (ch == '\n') ++c.lines;`. Then do the word logic from last lesson.
---
Printing:
`std::cout << std::format("{:>7} {:>7} {:>7}\n", c.lines, c.words, c.bytes);`
