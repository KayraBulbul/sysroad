`slot_for` is the whole idea in four lines:

```cpp
std::size_t mask = slots_.size() - 1;
std::size_t i = hash_(key) & mask;
while (slots_[i].state == State::Full && !(slots_[i].key == key))
    i = (i + 1) & mask;
return i;
```

It always ends, because the table is never full.
---
`find` asks `slot_for` and checks what it got:
`std::size_t i = slot_for(key); return slots_[i].state == State::Full ? &slots_[i].value : nullptr;`.
The const version is the same with `const V*`.
---
`insert`:

```cpp
if ((size_ + 1) * 4 > slots_.size() * 3) grow();
std::size_t i = slot_for(key);
if (slots_[i].state == State::Full) return false;   // already there
slots_[i] = Slot{State::Full, key, std::move(value)};
++size_;
return true;
```

Grow *before* finding the slot, because growing moves everything.
---
`operator[]`: `if (V* v = find(key)) return *v; insert(key, V()); return *find(key);`.
