**Why `include/textutil/textutil.hpp`, not `include/textutil.hpp`?**
The extra folder namespaces the header. Code writes
`#include "textutil/textutil.hpp"`, which can't clash with some other
library's `textutil.hpp`, or with a `util.hpp` the program has itself.
Boost (`boost/...`) and fmt (`fmt/format.h`) do the same.

**Static or shared?** `add_library(textutil ...)` with no keyword
builds a static library by default: an archive of `.o` files the linker
copies into each program. `add_library(textutil SHARED ...)` builds a
`.so` (`.dylib` on the Mac) that programs load when they start. The
cache variable `BUILD_SHARED_LIBS=ON` flips the default for every
library at once.

**Transitive, all the way down.** If `textutil` linked a `unicode`
library PUBLIC, `wc` would get `unicode`'s include folder too, and so
would anything that linked `wc`'s libraries. CMake works out the whole
chain. That's why `include_directories()` and friends are frowned on
now: they apply to every target in the folder, wanted or not, and you
can't tell from a target what it really needs.

**Bigger projects** give each folder its own `CMakeLists.txt` and the
top one calls `add_subdirectory(src)`. Targets are global, so a target
made in `src/` can be linked from `app/`. You'll do that in the
coreutils project.
