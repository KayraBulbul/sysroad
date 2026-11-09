**What's in `build/`?** `CMakeCache.txt` holds every setting CMake
worked out: the compiler's path, the build type, options. Configure
again and it reads the cache instead of starting over. `build.ninja`
(or `Makefile`) has the generated rules. Open it and you'll find the
`-MD -MF` dependency files from the last lesson, and a rule that re-runs
CMake when `CMakeLists.txt` changes.

**Why not `set(CMAKE_CXX_STANDARD 20)`?** It works, and you'll see it in
many projects. It sets the default for every target made after it.
`target_compile_features` attaches the need to one target, and as
you'll see next lesson, a library can pass it on to whoever uses it.
That's the "modern CMake" idea: describe targets, not global settings.

**`-std=gnu++20`, not `c++20`?** CMake turns on the compiler's GNU
extensions by default. Add `set(CMAKE_CXX_EXTENSIONS OFF)` before the
targets for strict `-std=c++20`. Portable code shouldn't depend on the
extensions anyway.

**Generators.** `cmake -G Ninja` picks Ninja, `-G "Unix Makefiles"`
make, `-G Xcode` an Xcode project. The same `CMakeLists.txt` drives
all of them. That's most of why C++ projects settled on CMake: one
description, every platform's tools.

**How sysroad reads your project back.** It asks CMake through its
File API: drop an empty file named `codemodel-v2` in
`build/.cmake/api/v1/query/`, configure, and CMake writes JSON describing
every target, its sources, flags and links. IDEs like CLion and VS Code
use the same API.
