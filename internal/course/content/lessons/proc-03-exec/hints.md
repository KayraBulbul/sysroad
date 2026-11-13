Parse in three stages with one index: options while the word starts
with `-`, then words that contain a `=` that isn't the first
character, then everything left is the command. `"--"` ends the
options and is dropped. `"-"` on its own means `-i`.
---
A name is everything before the first `=`:
`entry.substr(0, entry.find('='))`. Compare names, not prefixes: `A`
mustn't match `AB=1`.
---
Walk `PATH` between the colons with `find(':', start)`. For each
folder, `stat` the candidate: `S_ISREG(st.st_mode)` says it's a
regular file, and `access(p, X_OK) == 0` says you may run it.
---
```cpp
std::vector<char*> args;
for (const auto& s : argv) {
    args.push_back(const_cast<char*>(s.c_str()));
}
args.push_back(nullptr);
// the same for env, then:
::execve(program.c_str(), args.data(), envp.data());
```

`execve` takes `char* const[]` for historical reasons and never writes
to the strings, so the `const_cast` is safe. If `find_in_path` comes
back empty, set `errno = ENOENT` and return.
