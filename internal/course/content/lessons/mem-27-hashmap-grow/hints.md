`slot_for` stops only at an empty slot or a *full* slot holding the key:

```cpp
while (slots_[i].state != State::Empty &&
       !(slots_[i].state == State::Full && slots_[i].key == key))
    i = (i + 1) & mask;
```

A tombstone's key is junk, so check the state before comparing keys.
---
`erase`: `slot_for(key)`, and if that slot is `Full`, overwrite it with
`Slot{State::Deleted, K{}, V{}}` and decrement `size_`. Assigning a fresh
`Slot` frees whatever the old key and value owned. `used_` stays the
same, since the slot is still in the way of probes.
---
`insert` can't stop at the first tombstone, because the key might be
further along. So check `find(key)` first. Once you know the key is
absent, the first slot that isn't `Full` is a fine home:

```cpp
while (slots_[i].state == State::Full) i = (i + 1) & mask;
if (slots_[i].state == State::Empty) ++used_;   // a tombstone was already counted
```
---
`rehash` is the old `grow` with a size argument and `used_ = 0`, plus a
loop that doubles `n` until `(size_ + 1) * 4 <= n * 3`. In `insert`:

```cpp
if ((used_ + 1) * 4 > slots_.size() * 3)
    rehash((size_ + 1) * 2 > slots_.size() ? slots_.size() * 2 : slots_.size());
```

Then delete `grow`.
