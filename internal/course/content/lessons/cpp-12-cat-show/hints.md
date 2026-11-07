Replace the plain `text += ch;` with a chain:

```cpp
if (ch == '\n') { if (o.show_ends) text += '$'; text += '\n'; }
else if (ch == '\t') { text += o.show_tabs ? "^I" : "\t"; }
else if (o.show_nonprinting) { show_byte(text, static_cast<unsigned char>(ch)); }
else { text += ch; }
```
---
In `show_byte`, deal with the high bit first:
`if (c >= 128) { text += "M-"; c -= 128; }`. What's left is 0–127, which is
`^X`, `^?`, or the character itself.
---
For the held-back `\r`, check `st.pending_cr` first thing, before the
chain: if it's set, clear it and add `"^M"` when `ch == '\n'`, else a plain
`'\r'`. Then in the chain, before the tab case:
`else if (ch == '\r' && o.show_ends && !o.show_nonprinting) st.pending_cr = true;`
(with `-v`, `show_byte` already turns `\r` into `^M`).
