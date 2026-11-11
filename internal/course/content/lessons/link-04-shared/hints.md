`RTLD_NOW` resolves every symbol the library needs straight away, so
a missing one fails here rather than in the middle of a call later.
Add `RTLD_LOCAL` so the plugin's symbols don't become visible to
libraries loaded after it.

```cpp
handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
if (handle_ == nullptr) {
    throw std::runtime_error(dlerror());
}
```
---
The moves are `unique_ptr`'s, with `dlclose` for `delete`:

```cpp
Library::Library(Library&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}
```

Move assignment: if it isn't a self-move, close your own handle (if
any), then take the other's with `std::exchange`.
---
A symbol's address can legitimately be null, so `dlsym` returning
`nullptr` doesn't prove it failed. Clear the error first, then check
`dlerror()` afterwards:

```cpp
dlerror();
void* p = dlsym(handle_, name.c_str());
if (const char* err = dlerror()) {
    throw std::runtime_error(err);
}
return p;
```
