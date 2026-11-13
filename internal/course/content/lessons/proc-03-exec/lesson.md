---
title: exec and the environment
minutes: 35
files: [env.cpp]
program: env
run:
  args: ["-i", "PATH=/usr/bin:/bin", "GREETING=hello from exec", "sh", "-c", "echo \"$GREETING\"; env"]
deeper:
  - title: "CS:APP 8.4.5-8.4.6: loading and running programs"
    url: https://csapp.cs.cmu.edu/
  - title: "execve(2) man page"
    url: https://man7.org/linux/man-pages/man2/execve.2.html
  - title: "POSIX env, and how PATH is searched"
    url: https://pubs.opengroup.org/onlinepubs/9799919799/utilities/env.html
---
`fork` makes a copy of the program that's running. To run a
*different* program, a process calls `execve`, and becomes it:

```cpp
char* argv[] = {(char*)"ls", (char*)"-l", nullptr};
char* envp[] = {(char*)"PATH=/usr/bin:/bin", nullptr};
execve("/bin/ls", argv, envp);
// only reached if it failed: errno says why
```

The kernel throws away the process's memory, loads `/bin/ls` in its
place, and starts it at its `main` with that `argv` and environment.
It's still the same process: the same PID, the same parent, the same
open fds, the same current directory. Nothing returns, because the
code that called `execve` is gone.

So every program you start from a shell is two steps: `fork` to get a
second process, then `exec` in the child to turn it into the program.
In between, the child can rearrange its fds, change folders or tweak
its environment, and the program it becomes inherits all of it.
That's the whole trick behind redirection, pipes, `cd`, and `env`.

Notice `argv[0]` and the path are separate. `argv[0]` is just the
name the program sees for itself; the path is the file the kernel
loads. Both arrays are C arrays of C strings, ending in a null
pointer, so a `std::vector<std::string>` has to become a
`std::vector<char*>` with a `nullptr` on the end. Make it before
calling `execve`, and keep the strings alive until then.

`execve` wants a path. Shells search for the program in the folders
listed in `PATH`, in order, and take the first **executable regular
file** they find. A name with a `/` in it isn't searched for. An empty
entry in `PATH` (a leading or trailing `:`, or `::`) means the current
folder. The C library's `execvp` does this search for you, using
*this* process's `PATH`. You'll do it yourself, with the `PATH` from
the new environment.

That's `env`, the program at the top of countless scripts
(`#!/usr/bin/env python3`). It edits its own environment and then
`exec`s the command:

```sh
env -i PATH=/usr/bin:/bin GREETING=hi sh -c 'echo $GREETING'
```

`-i` starts from an empty environment, and `-u NAME` removes one
variable. With no command, `env` prints the environment. If the
command can't be found it exits with 127, and if it's found but won't
run, 126: the same codes your shell uses.

## Assignment

- [ ] `parse_env_args` reads the options (`-i` or `-`, `-u NAME`, `--`), then the `NAME=VALUE`s, then the command
- [ ] `build_environment` applies `-i`, then the `-u`s, then each `NAME=VALUE`, replacing in place or adding at the end
- [ ] `find_in_path` returns the first executable regular file on the `PATH`, treating an empty entry as `.`
- [ ] `exec_with` finds the program through the new environment's `PATH` (`/usr/bin:/bin` if it has none) and `execve`s it
