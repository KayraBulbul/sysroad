---
title: How a lesson works
minutes: 5
xp: 5
files: [add.cpp]
---
Every lesson has a folder of starter code in your data repo. It syncs
between your machines, so you can stop on the PC and carry on on the Mac.

```text
sysroad path          prints this lesson's folder
sysroad edit          opens the file you work in, in $EDITOR
```

Keep your editor in one split and `sysroad side` in the other. The sidebar
shows the lesson, and the results of every Run and Submit, even ones you
start from your editor's shell.

```text
r   Run       build, run the visible tests, stop at the first failure
s   Submit    run every test, including hidden ones; a pass completes it
h   hint      reveal the next hint
v   solution  peek at the solution (it asks first)
R   reset     put the starter code back (your version is kept in .local/)
```

Each command also works from a shell: `sysroad run`, `sysroad submit`,
`sysroad hint`, `sysroad solution`, `sysroad reset`. Run from inside a
lesson's folder, they act on that lesson.

Once you pass, an **Under the hood** section unlocks with how the real
thing works.

## Assignment

`add.cpp` has a bug: `add` subtracts.

- [ ] Run, and read the failing test
- [ ] Fix `add`, Run again, then Submit
