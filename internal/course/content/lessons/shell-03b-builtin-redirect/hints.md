The class keeps pairs of {fd, its copy}, with -1 for "it was closed":

```cpp
class SavedFds {
  public:
    explicit SavedFds(const std::vector<Redirect>& redirects);
    ~SavedFds();                 // dup2(copy, fd); close(copy), or close(fd) for -1
    SavedFds(const SavedFds&) = delete;
    SavedFds& operator=(const SavedFds&) = delete;
  private:
    std::vector<std::pair<int, int>> saved_;
};
```

Restore in the reverse order you saved.
---
In the constructor, work out `above`: one more than the highest fd the
redirections name, `fd` or the fd they copy from. Then for each fd not
saved yet:

```cpp
int copy = fcntl(fd, F_DUPFD_CLOEXEC, std::max(10, above));
if (copy == -1 && errno != EBADF) copy = fcntl(fd, F_DUPFD_CLOEXEC, above);
if (copy == -1 && errno != EBADF) { /* close the copies made so far, throw */ }
saved_.emplace_back(fd, copy);   // -1 when it was closed (EBADF)
```
---
Running the builtin: make the saver first, then apply, then run. A
`std::optional<SavedFds>` lets you construct it inside a `try` and still
have it live after:

```cpp
std::optional<SavedFds> saved;
try {
    saved.emplace(c.redirects);
    apply_redirects(c.redirects);
} catch (const std::system_error& e) {
    std::cerr << "conch: " << e.what() << '\n';
    return 1;   // saved's destructor undoes what was applied
}
```
