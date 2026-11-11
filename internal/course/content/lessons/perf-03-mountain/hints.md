`read_stride` visits 0, stride, 2 × stride, … while the index is below
`elems`. The four-sum loop stops when there aren't four left, so
finish with one more loop from where it stopped:

```cpp
for (; i < elems; i += stride) {
    a += data[i];
}
return a + b + c + d;
```

`bytes_read` is that count of indices times `sizeof(long)`. It's
`elems / stride` rounded up, so `(10, 3)` is 4.
---
`human_size` divides by 1024 while the number is at least 1024 and
divides exactly, counting how many times, then adds `"K"`, `"M"`, `"G"`
or `"T"` for 1, 2, 3 or 4 divisions. `std::to_string` makes the
number a string.
---
`build_mountain`: start with `Mountain m{sizes, strides, {}}`. For each
size, `size / sizeof(long)` is the number of elements. For each stride:

```cpp
double ns = time([&] { do_not_optimize(read_stride(data.data(), elems, stride)); });
row.push_back(static_cast<double>(bytes_read(elems, stride)) / ns);
```

Push each finished row onto `m.gbps`.
