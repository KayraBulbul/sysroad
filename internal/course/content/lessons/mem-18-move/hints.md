The lesson shows the move constructor. Move assignment follows the same
idea with a self-check:

```cpp
Fd& Fd::operator=(Fd&& other) noexcept {
    if (this != &other) {
        reset(other.release());
    }
    return *this;
}
```

`release()` empties `other` and hands back its fd, and `reset` closes
the old one and keeps the new one. Both are from 1.3.4.
---
`open_all`:

```cpp
std::vector<Fd> out;
for (const std::string& p : paths) {
    Fd fd = open_read(p.c_str());
    if (fd.valid()) out.push_back(std::move(fd));
    else failed.push_back(p);
}
return out;
```

Without the `std::move`, `push_back` would try to copy, and the deleted
copy constructor stops that at compile time.
---
`open_first` returns a named local, which moves it:
`Fd fd = open_read(p.c_str()); if (fd.valid()) return fd;`. After the
loop, `return Fd();` (an empty one).
