**GNU grep doesn't read lines.** It reads big buffers and runs the
pattern search over the whole buffer at once. Only when it finds a match
does it look backwards and forwards for the line's edges. Searching first
and splitting lines later is a large part of why grep is so fast.

**Fixed-string search is a solved problem.** `std::string::find` is a
fairly simple loop (libstdc++ uses `memchr` for the first character and
then compares). GNU grep uses Boyer–Moore-style algorithms, which skip
ahead by up to the pattern's length when a character can't be part of a
match, so long patterns make it *faster*. `std::boyer_moore_searcher`
exists in `<functional>` if you want to try it.

**grep is quiet when output goes nowhere.** If stdout is `/dev/null`,
GNU grep notices (with `fstat`) and stops at the first match, as if you'd
passed `-q`. It's a real optimisation for `if grep ... > /dev/null`
scripts.

**Regular expressions come later.** Phase 11 builds an interpreter: a
lexer, a parser and an evaluator. A small regex engine (Thompson's
construction, from Russ Cox's articles) is a great side project after that.
