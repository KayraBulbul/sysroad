**sysroad didn't download anything.** The engine sets
`FETCHCONTENT_SOURCE_DIR_DOCTEST` to a copy of doctest it carries, so
your build works offline and in a second. Any FetchContent dependency
has that override: point `FETCHCONTENT_SOURCE_DIR_<NAME>` at a local
checkout to work on a library and the program using it together.

**`URL` with a hash beats `GIT_REPOSITORY`.** Cloning is slower than
downloading a release tarball, and a tag can be moved. This is safer:

```cmake
FetchContent_Declare(doctest
  URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz
  URL_HASH SHA256=<the tarball's sha256>)
```

If the download ever changes, the build stops instead of compiling
someone else's code.

**`find_package` first.** The other half of CMake's story is
`find_package(fmt)`, which uses a copy already installed on the
machine, by `pacman` or `brew` say. Since CMake 3.24,
`FetchContent_Declare(... FIND_PACKAGE_ARGS)` tries `find_package` and
only downloads when that fails: distro packagers get their system
libraries, and everyone else gets a working build.

**And the rest.** vcpkg and Conan are package managers that build
libraries ahead of time and hand CMake the results through
`find_package`. They scale better for dozens of dependencies. For a
handful, FetchContent is the simplest thing that works.

**`SYSTEM`.** With CMake 3.25 or newer, `FetchContent_Declare(...
SYSTEM)` marks the dependency's headers as system headers, so your
`-Wall -Wpedantic` doesn't report warnings in code you didn't write.
