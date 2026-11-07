`-v` is one comparison at the end: `return found != o.invert;`. If
`invert` is false you get `found`. If it's true you get the opposite.
---
Count every line you read, matched or not (`++n` at the top of the loop),
so `-n` shows the line's real number.
---
For `-c`, skip printing inside the loop (`if (o.count) continue;` after
counting), and after the loop do
`if (o.count) out << prefix << selected << '\n';`.
