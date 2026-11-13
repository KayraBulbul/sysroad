`readdir` returns a pointer to a `dirent`, or null at the end, and
`entry->d_name` is the file's name as a C string. Get the folder's own
fd before the loop, and skip it along with `.` and `..`:

```cpp
DIR* dir = opendir("/dev/fd");
int own = dirfd(dir);
while (dirent* entry = readdir(dir)) {
    std::string name = entry->d_name;
    // ...
}
closedir(dir);
```

`std::stoi(name)` turns `"57"` into 57. Folders don't come back in any
particular order, so sort at the end.
---
`environ` ends with a null pointer, like `argv`:

```cpp
for (char** p = environ; *p != nullptr; ++p) {
    std::string entry = *p;
    auto eq = entry.find('=');   // the first one: values can contain '='
    // ...
}
```
---
`getcwd(buf, sizeof buf)` fills `buf` and returns it, or returns null
if the path doesn't fit. A 4096-byte buffer (`PATH_MAX` on Linux) is
plenty here.
