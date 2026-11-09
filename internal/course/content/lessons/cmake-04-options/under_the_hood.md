**Why `FORCE`, and why check `CMAKE_CONFIGURATION_TYPES`?** A cache
variable that already exists, even empty, isn't replaced by `set(...
CACHE ...)` unless you `FORCE` it. And generators like Xcode and Visual
Studio are *multi-config*: one build folder holds Debug and Release
side by side, and you pick at build time with `cmake --build build
--config Release`. There, `CMAKE_BUILD_TYPE` means nothing, so the
default is skipped.

**Generator expressions.** `$<CONFIG:Debug>` is true when building the
Debug configuration, so `target_compile_definitions(geo PRIVATE
$<$<CONFIG:Debug>:GEO_CHECKS=1>)` adds a define to Debug builds only.
They're worked out at build time rather than configure time, which is
why they work on multi-config generators where an `if()` can't.

**RelWithDebInfo** is `-O2 -g`: optimised, but with debug info, so a
crash in production still has a readable backtrace. It's what most
Linux distributions ship, then they strip the debug info into a
separate package.

**Presets.** Typing `-DSANITIZE=ON -DCMAKE_BUILD_TYPE=Debug -G Ninja`
gets old. A `CMakePresets.json` next to `CMakeLists.txt` names those
combinations: `cmake --preset asan`. It's checked into the repo, so
everyone's builds match.

**`-Werror` in CI, not on laptops.** A new compiler adds new warnings,
and with `-Werror` a release that built fine yesterday stops building.
A common setup is an option, off by default, that CI turns on.
