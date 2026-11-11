---
title: Shared libraries and the dynamic loader
minutes: 40
files: [library.cpp]
program: plugins
run:
  args: ["systems programming"]
deeper:
  - title: "CS:APP 7.10-7.12: dynamic linking and position-independent code"
    url: https://csapp.cs.cmu.edu/
  - title: "How To Write Shared Libraries (Ulrich Drepper)"
    url: https://www.akkadia.org/drepper/dsohowto.pdf
  - title: "dlopen(3) man page"
    url: https://man7.org/linux/man-pages/man3/dlopen.3.html
---
Every program on your machine calls `printf`, and none of them carries
a copy. They share one: `libc.so.6` on Linux, `libSystem.B.dylib` on
the Mac. A **shared library** is linked when the program *starts*, by
the **dynamic loader** (`ld-linux-x86-64.so.2`, or `dyld` on the Mac),
not when it's built. Its code is in memory once, however many
programs use it, and a fixed version reaches every program without
rebuilding any of them.

```sh
clang++ -shared -fPIC add.cpp -o libadd.so     # .dylib by convention on the Mac
clang++ main.cpp -L. -ladd -o prog
ldd prog              # Linux: which libraries, found where
otool -L prog         # the Mac
```

At link time the linker only notes that `prog` *needs* `libadd.so`
and which symbols come from it. At start-up the loader finds the file
(in the program's **rpath**, `LD_LIBRARY_PATH`, then the system
folders), maps it into memory at whatever address is free, and fills
in the addresses of the symbols the program uses.

"Whatever address is free" is why the code must be
**position-independent** (`-fPIC`): it can't contain absolute
addresses. Calls into other libraries go through small stubs in the
**PLT** (procedure linkage table), which jump to an address the loader
writes into the **GOT** (global offset table) once it knows it. Your
program's own code reaches the GOT relative to the instruction
pointer, so it doesn't matter where it was loaded.

A program can also load a library itself, at any time, by name. That's
how plugins work: Python extension modules, audio plugins, database
drivers, your editor's language servers.

```cpp
void* h = dlopen("./libplugin.so", RTLD_NOW);       // nullptr on failure
auto f = (int (*)(int)) dlsym(h, "plugin_apply");  // nullptr if not found
dlclose(h);
```

The errors aren't exceptions: `dlerror()` returns a message string
for the last failure. `dlsym` looks up a *symbol*, so the plugin's
functions are `extern "C"`, which turns name mangling off.

`dlopen` and `dlclose` are a resource pair like `new` and `delete`, so
wrap them the way you wrapped those in phase 2: a move-only `Library`
that closes its handle in its destructor. Run builds two plugins with
the compiler, loads them through your class, and first asks the loader
which file some functions came from.

## Assignment

- [ ] The constructor `dlopen`s with `RTLD_NOW`, and throws `std::runtime_error` with `dlerror()`'s message if it fails
- [ ] The destructor `dlclose`s, unless the handle was moved away
- [ ] The move constructor and move assignment hand over the handle, closing any old one
- [ ] `symbol` throws for a missing symbol or a moved-from Library
- [ ] Run `ldd` or `otool -L` on any program on your machine
