A `std::vector<std::string>` in your `Shell`, and one function that
adds a line by the rules: return early for a line with nothing but
spaces and tabs (`line.find_first_not_of(" \t") == npos`) or one equal
to `history.back()`; push it; if there are more than 1000, erase the
first.
---
`history N`: check the count is all digits (and not absurdly long)
before `std::stoul`, then start from `size() - N` if that's above 0.
`std::snprintf(buf, sizeof buf, "%5zu", i + 1)` makes the number.
---
Loading: `std::ifstream` and `std::getline`, adding each line through
the same function. A file that isn't there gives no lines, which is
fine.
---
Saving: `open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0600)`, then
`fchmod(fd, 0600)` (a `.tmp` left over from before keeps its old bits
otherwise), `write` every line with its `\n`, `close`, and
`std::rename(tmp, path)`. If a write fails, close and keep the old
file. In `main`, save after the loop, so `exit` and the end of input
both get there: let `exit` end the loop rather than return from
`main`.
