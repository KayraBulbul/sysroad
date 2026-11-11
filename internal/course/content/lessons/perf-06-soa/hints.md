`add` is nine `push_back`s. The arrays in `Cold` can't be assigned
with `=`, so copy them with `std::memcpy`:

```cpp
Cold c;
std::memcpy(c.name, p.name, sizeof c.name);
std::memcpy(c.color, p.color, sizeof c.color);
c.spawned_at = p.spawned_at;
cold_.push_back(c);
```

`get` does the same in reverse into a `Particle`. Every vector has the
same length, so `size` can return any of their sizes.
---
`step` is the loop from `aos.cpp` with `p.vz` written as `vz_[i]` and
so on:

```cpp
for (std::size_t i = 0; i < size(); ++i) {
    vz_[i] -= gravity * dt;
    x_[i] += vx_[i] * dt;
    // y and z the same way
}
```

Keep the order: `vz` changes first, and the new `vz` moves `z`.
