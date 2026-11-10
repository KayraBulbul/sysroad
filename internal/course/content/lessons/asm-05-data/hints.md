`kilo` adds up `n` 8-byte values starting at `a`. `lima` loads two
4-byte values, at `a + 4*i` and at `a + 4*i + 8`, which are `a[i]`
and `a[i + 2]`, and adds them. `mike` loads `*b` and adds it into
`*a`. It returns nothing.
---
`Item`'s layout, from `oscar`: it loads 4 bytes at offset 0, 2 bytes
at offset 4 and 1 byte at offset 6. `Item` has an `int` to spare, one
`short` and one `char`, and the description says they're `id`,
`shelf` and `tag`.

From `november`: it starts by adding 16 to the pointer. Then it loads
4 bytes at `(%rdi)`, which is offset 16, and multiplies by the 8
bytes at `-8(%rdi)`, which is offset 8. The description says those
are `count` and `price`. Each trip adds 24, so `sizeof(Item)` is 24.
---
So the order is `id`, `shelf`, `tag`, `price`, `count`. Check it
against lesson 2.1.2: `int` at 0, `short` at 4, `char` at 6, one byte
of padding, `long` at 8, `int` at 16, and 4 bytes of padding at the
end to make 24.
---
With the struct right, the two functions are short:

```cpp
total += items[i].price * items[i].count;      // november, in a loop over n
return item->id + item->shelf + item->tag;     // oscar
```
---
`papa`: `Node` is given, with `key` at 0, `value` at 8 and `next` at
16. The loop compares `node->key` with `key`. On a match it returns
`node->value`. Otherwise it moves to `node->next`, and if it runs out
of nodes it returns the -1 that was put in `rax` (`x0`) for that
case.
