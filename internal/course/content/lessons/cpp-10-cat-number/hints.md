A line is empty exactly when its first byte is `'\n'`. So at the start of
a line, `-b` numbers it if `ch != '\n'`, and `-n` always does.
---
```cpp
if (st.at_line_start) {
    bool numbered = o.number_nonblank ? ch != '\n' : o.number;
    if (numbered) text += std::format("{:>6}\t", ++st.line);
}
st.at_line_start = ch == '\n';
```
