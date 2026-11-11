`transpose_blocked` is four loops. The outer two step `ii` and `jj` by
`block`. The inner two run `i` from `ii` and `j` from `jj`, stopping
at the tile's edge or the matrix's, whichever comes first:

```cpp
std::size_t i_end = std::min(ii + block, a.rows());
std::size_t j_end = std::min(jj + block, a.cols());
```

The body is the same as `transpose`'s: `out(j, i) = a(i, j);`
---
`multiply_ikj` first sets every `c(i, j)` to 0. Then:

```cpp
for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t k = 0; k < n; ++k) {
        double aik = a(i, k);
        for (std::size_t j = 0; j < n; ++j) {
            c(i, j) += aik * b(k, j);
        }
    }
}
```
