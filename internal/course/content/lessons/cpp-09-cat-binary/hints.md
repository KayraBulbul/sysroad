`in.gcount()` returns a `std::streamsize`, which is signed. Keep a running
`std::size_t total` and add `static_cast<std::size_t>(in.gcount())` each
time round the loop.
