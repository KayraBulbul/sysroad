A `std::map` from name to function makes `find_builtin` a lookup:

```cpp
Builtin find_builtin(const std::string& name) {
    static const std::map<std::string, Builtin> builtins = {
        {"cd", builtin_cd}, {"pwd", builtin_pwd}, // ...
    };
    auto it = builtins.find(name);
    return it == builtins.end() ? nullptr : it->second;
}
```
---
For `cd`, remember the old folder before you `chdir`, so you can set
`OLDPWD` only if the change worked:

```cpp
char before[PATH_MAX];
bool have_before = getcwd(before, sizeof before) != nullptr;
if (chdir(dir.c_str()) == -1) {
    std::cerr << "conch: cd: " << dir << ": " << std::strerror(errno) << '\n';
    return 1;
}
```
---
`std::strtol(s, &end, 10)` parses a number and leaves `end` at the
first character it didn't use: the number was good if `*end == '\0'`
and the string wasn't empty. `n & 0xff` keeps the low 8 bits.
