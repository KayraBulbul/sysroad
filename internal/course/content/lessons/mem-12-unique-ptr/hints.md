`std::lower_bound` finds where a name is, or where it would go, in a
sorted vector:

```cpp
auto it = std::lower_bound(parent.children.begin(), parent.children.end(), name,
    [](const std::unique_ptr<Entry>& e, std::string_view n) { return e->name < n; });
if (it != parent.children.end() && (*it)->name == name) return it->get();
auto e = std::make_unique<Entry>();
e->name = name;
return parent.children.insert(it, std::move(e))->get();
```

`insert` returns an iterator to the new element, and `->get()` turns the
`unique_ptr` into a plain `Entry*`.
---
`add_path`: walk the path a part at a time, moving a cursor down:

```cpp
Entry* at = &root;
while (!path.empty()) {
    std::size_t slash = path.find('/');
    std::string_view part = path.substr(0, slash);
    path = slash == std::string_view::npos ? "" : path.substr(slash + 1);
    if (!part.empty() && part != ".") at = child(*at, part);
}
```
---
`detach` finds the child the same way, then
`std::unique_ptr<Entry> out = std::move(*it); parent.children.erase(it); return out;`.
After the move, the slot in the vector holds a null pointer, which
`erase` removes.
---
For `print_tree`, a recursive helper takes the prefix built so far. The
last child gets `└── ` and passes `prefix + "    "` down. The others get
`├── ` and pass `prefix + "│   "`. Count an entry as a directory if it
has children and as a file otherwise. The summary is
`"\n" + dirs + " directories, " + files + " files"`, with
`directory` and `file` when the count is 1.
