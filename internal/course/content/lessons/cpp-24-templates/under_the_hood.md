**Every instantiation is separate machine code.** `Stack<int>` and
`Stack<std::string>` share no code at run time, so templates can make
binaries bigger ("code bloat"). In exchange, each one is specialised for its
type, with `int` comparisons inlined as single instructions. `nm -C prog |
grep Stack` lists the instantiations. Phase 5 covers symbols.

**Duck typing at compile time.** Before C++20, a template accepted any
type and only failed when an operation didn't exist, deep inside the body.
Concepts move the check to the front door. You can write your own:
`template <typename T> concept Sized = requires(T t) { t.size(); };`

**Why errors used to be famous.** Get a `std::map` key type wrong in
C++03 and the error was a few hundred lines of nested template names.
Concepts, and better compilers (clang especially), have made them much
shorter.

**Templates are a language in themselves.** You can compute at compile
time with them (template metaprogramming), and `if constexpr` lets one
template take different code paths for different types. Most code never
needs that, but the standard library uses it everywhere, which is why its
headers are so dense.
