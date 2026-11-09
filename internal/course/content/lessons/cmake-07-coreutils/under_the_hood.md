**`GNUInstallDirs`.** `bin` is right almost everywhere, but `lib`
isn't: Fedora puts 64-bit libraries in `lib64`, Debian in
`lib/x86_64-linux-gnu`. `include(GNUInstallDirs)` defines
`CMAKE_INSTALL_BINDIR`, `CMAKE_INSTALL_LIBDIR` and friends with the
platform's answer, and installing a library should use them.

**Installing a library for others.** Installing the `.a` and its
headers is half the job. The other half is a *package config* file, so
another project's `find_package(coreutils)` finds it and gets a
`coreutils::coreutils` target with the right include path. That's
`install(EXPORT ...)` plus `configure_package_config_file`. It's
fiddly, which is part of why FetchContent is popular.

**`DESTDIR`.** Packagers run `DESTDIR=/tmp/pkg cmake --install build`
to install into a staging folder that mirrors `/`, then archive it.
That's how an Arch `PKGBUILD` or a Homebrew formula uses your
`install()` rules.

**Why each folder gets its own `CMakeLists.txt`.** Paths stay short and
local, each folder reads on its own, and moving a folder moves its
build with it. Variables set in a folder stay in that folder and the
ones below it, while targets are visible everywhere. Folders hold the
details, and targets are how they talk to each other.
