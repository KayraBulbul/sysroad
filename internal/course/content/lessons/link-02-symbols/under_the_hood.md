**Where `inline` functions go.** An inline function that isn't
inlined everywhere still needs an out-of-line copy. The compiler puts
it in its own section marked as a **COMDAT** group (on ELF;
"coalesced" or *weak definition* on Mach-O), and the linker keeps the
first copy it sees and drops the rest without checking they're the
same. If two files compile `mean` differently (say, with different
`#define`s in force), you get one of them, silently. That's an ODR
violation that no tool is required to report. Templates work the same
way: every file that uses `std::vector<double>` instantiates its
members, and the linker deduplicates.

**Weak symbols.** `nm` shows them as `W` or `V`. A weak definition
loses to a strong one without an error, which lets a library ship a
default that a program can replace by defining the same name. C++
lets you replace `operator new` and `operator delete` like that,
which phase 7 does.

**What an anonymous namespace does to the symbol.** Its contents get
internal linkage: in `nm` they show as lower-case `t`, and the linker
never matches them against other files. The compiler is then also
free to inline them everywhere and drop the out-of-line copy, since
nothing outside the file can call it. Unnamed namespaces are the C++
way; `static` on a function is the C way and means the same.

**`extern "C"`.** It turns mangling off: `extern "C" int add(int, int)`
is the plain symbol `add` (`_add` on the Mac, which prefixes every C
symbol with an underscore). That's how C++ calls C libraries and how
C, Python's `ctypes` or `dlsym` can find a C++ function. The price is
no overloading: two `extern "C"` functions can't share a name.

**Reading mangled names.** `c++filt _Z8smallestRKSt6vectorIdSaIdEE`
prints `smallest(std::vector<double, std::allocator<double> > const&)`.
`_Z` starts a mangled name, `8smallest` is a length-prefixed name,
`R` is a reference, `K` const, `St` the `std::` namespace, `I…E` a
template argument list, `d` double. MSVC on Windows mangles
completely differently (`?smallest@@YANAEBV?$vector@N…`), one reason
C++ libraries can't be mixed between compilers.
