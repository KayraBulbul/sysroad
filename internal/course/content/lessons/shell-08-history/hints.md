`feed` is one `switch` on the key. `std::string::insert(cursor_, 1,
c)` and `erase(pos, n)` do the editing; after an insert, move the
cursor right. Ctrl-letter keys are `ctrl_key('a')` and so on, from
`keys.hpp`.
---
History browsing needs two members, already in the header:
`browsing_`, the index shown (`SIZE_MAX` while you're on the new
line), and `draft_`. The first Up saves `text_` in `draft_` and starts
at `history_->size()`; each Up moves one older; Down moves newer, and
past the end puts `draft_` back.
---
```cpp
std::string out = "\r" + prompt + text_ + "\x1b[K";
if (cursor_ < text_.size()) {
    out += "\x1b[" + std::to_string(text_.size() - cursor_) + "D";
}
```
---
`save_history`: write every line to `path + ".tmp"`, close it, and
`std::rename(tmp.c_str(), path.c_str())`. `load_history` reads with
`std::getline` and calls `add_history` on each line, so the same rules
apply.
