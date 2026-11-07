**How ASan knows.** For every 8 bytes of your memory, ASan keeps one
"shadow" byte that says how many of them are valid. The compiler adds a
check before every load and store. `malloc` is replaced by one that puts
poisoned *redzones* around each block, and `free` poisons the block and
holds it in quarantine for a while, so a later access is caught instead of
landing in a reused block. Programs run about 2× slower and use about 3×
the memory.

**Undefined behaviour is worse than "a wrong answer".** The compiler
assumes UB never happens and optimises on that basis. `if (a + 1 < a)`,
an overflow check written the obvious way, can be deleted entirely at
`-O2`, because signed overflow "can't" happen. UBSan catches the overflow
itself at run time.

**Valgrind is the other tool.** It runs your unmodified binary on a
simulated CPU, so it catches similar bugs with no recompiling, but it's
20–50× slower and doesn't support recent macOS. ASan is the default for
code you can rebuild. `-fsanitize=thread` (TSan) catches data races, and
you'll turn it on in the concurrency phase.

**Bug 4 is the classic C++ trap.** Returning a `string_view`, a
reference, or a pointer to something local compiles cleanly, and clang
only warns about the most obvious cases. Sanitizers, a clear rule
(views never outlive what they look at), and returning owning types like
`std::string` are the defence.
