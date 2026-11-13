Keep one variable from one turn of the loop to the next: the read end
of the previous pipe, or -1 for the first command.

```cpp
int prev_read = -1;
for (size_t i = 0; i < commands.size(); ++i) {
    int p[2] = {-1, -1};
    if (i + 1 < commands.size()) {
        pipe(p);   // the last command writes to our stdout instead
    }
    // fork, and in the child: prev_read onto 0, p[1] onto 1
    // ...
}
```
---
In the child, after the `dup2`s, close every pipe fd it still has:
`prev_read`, `p[0]` and `p[1]`. Only 0, 1 and 2 matter now.
---
In the parent, after each fork:

```cpp
if (prev_read != -1) close(prev_read);   // command i has it now
if (p[1] != -1) close(p[1]);             // command i writes to it, not us
prev_read = p[0];                        // command i + 1 will read it
```

Wait for the PIDs in order only after the loop.
