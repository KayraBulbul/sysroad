**Exceptions cost nothing until they're thrown.** Modern compilers use
"zero-cost" exception handling. The happy path has no extra instructions,
and the unwinding information lives in a separate table (`.eh_frame`, which
you can see with `readelf -S`). Throwing is slow, though, around
microseconds, because the runtime walks those tables frame by frame. So:
fine for "this config is broken", wrong for "this line didn't match".

**Catch by reference.** `catch (ConfigError e)` would copy the exception,
and catching a base class by value *slices* it: a `ConfigError` caught as
a plain `std::exception` loses its extra parts. `catch (const T&)` avoids
both.

**`noexcept`** tells the compiler a function never throws. If it throws
anyway, the program calls `std::terminate`. Destructors are `noexcept` by
default, which is why throwing from a destructor during unwinding ends the
program. Move constructors marked `noexcept` let `std::vector` move instead
of copy when it grows (phase 2).

**Many codebases turn exceptions off.** Google, game engines and most
embedded code build with `-fno-exceptions`, and report errors with values
(`std::optional`, `std::expected`, status codes). The standard library
then calls `abort` where it would have thrown. You'll meet both styles.
This course uses values for expected failures and exceptions for "can't
continue".
