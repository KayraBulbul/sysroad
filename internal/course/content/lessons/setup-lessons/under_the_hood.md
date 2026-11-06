**What Run and Submit actually do.** Your code is compiled twice, in
parallel:

1. A normal build with `-Wall -Wextra`. Its warnings are the ones you see.
2. A test build: your files plus the lesson's test files, linked against a
   pre-built copy of [doctest](https://github.com/doctest/doctest)'s `main()`.
   If your file has a `main`, it's renamed with `-Dmain=sysroad_student_main`,
   so doctest's `main` runs instead.

Run builds only the visible tests and passes `--abort-after=1`. Submit adds
the `hidden_*.cpp` tests and checks your program's output against more
cases. That's how boot.dev's Run and Submit split works too, just with
`clang++` instead of Pyodide.

Builds go to `.local/build/` inside your data repo. It's gitignored, so only
your source code syncs.
