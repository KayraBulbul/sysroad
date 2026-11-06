`clang++ --version` should print a version of 17 or newer. On the Mac that's
Apple clang from the Command Line Tools.
---
If the sanitizer build fails on Linux with a linker error about `libclang_rt`,
install `compiler-rt`: `sudo pacman -S compiler-rt`.
