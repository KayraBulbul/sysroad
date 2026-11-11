`sum2`: step `i` by 2 while there are two left, then pick up the last
element if `n` is odd:

```cpp
double s0 = 0, s1 = 0;
std::size_t i = 0;
for (; i + 2 <= n; i += 2) {
    s0 += a[i];
    s1 += a[i + 1];
}
for (; i < n; ++i) {
    s0 += a[i];
}
return s0 + s1;
```

`sum4` is the same with four.
---
`poly_split` walks down from the top in steps of 2. If `n` is odd, the
top coefficient `a[n - 1]` has an even index and no odd partner, so
start `even` at it:

```cpp
double x2 = x * x, even = 0, odd = 0;
std::size_t i = n;
if (i % 2 == 1) {
    --i;
    even = a[i];
}
```

Then, while `i >= 2`: `odd = a[i - 1] + x2 * odd;`,
`even = a[i - 2] + x2 * even;`, `i -= 2`. The answer is
`even + x * odd`.
