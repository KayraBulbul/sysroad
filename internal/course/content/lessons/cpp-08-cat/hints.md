No arguments means the same as one `-`:

```cpp
std::vector<std::string_view> files = args;
if (files.empty()) files.push_back("-");
```
---
Check for a directory before opening:

```cpp
namespace fs = std::filesystem;
std::error_code ec;
if (fs::is_directory(fs::path(name), ec)) {
    err << "cat: " << name << ": Is a directory\n";
    status = 1;
    continue;
}
```
---
Then open with `std::ifstream f{std::string(name), std::ios::binary};`,
report `std::strerror(errno)` if `!f`, and otherwise `cat_lines(f, out);`.
