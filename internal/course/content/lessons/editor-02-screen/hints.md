Build the string with `+=`. A loop over `y` from 0 to `screen_rows`
decides what each row is: `y < rows.size()` is text, and the rest are
`~`, apart from the welcome row.
---
For the welcome, cut first: `kWelcome.substr(0, cols)`. The padding is
`(cols - welcome.size()) / 2`. If it's more than zero, write `~`, then
`padding - 1` spaces (`out.append(n, ' ')`), then the message.
---
`std::format("\x1b[{};{}H", e.cy + 1, e.cx + 1)` makes the cursor move.
Terminal rows and columns count from 1, and kite's from 0.
