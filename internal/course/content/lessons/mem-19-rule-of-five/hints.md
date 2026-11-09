The rule-of-zero fix is the short one. In `matrix.hpp`, replace
`double* data_;` with `std::vector<double> data_;` (and include
`<vector>`). Then in `matrix.cpp`, the constructor becomes
`data_(rows * cols, 0.0)` and the destructor goes away: delete its
declaration and its definition.
---
`at` keeps working unchanged: `data_[r * cols_ + c]` indexes a vector
just like a pointer.
---
If you'd rather write all five, start from `Buffer`'s copy constructor
and assignment (lesson 2.3.1) and `Fd`'s moves (2.3.3). In the moves,
take `rows_`, `cols_` and `data_`, then set the source's `data_` to
`nullptr` and its sizes to 0.
