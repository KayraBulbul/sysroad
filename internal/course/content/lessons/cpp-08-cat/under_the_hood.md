**GNU cat doesn't use lines at all.** With no flags it runs a tight loop
of `read(2)` into a buffer, then `write(2)` of whatever came back. It even
tries `copy_file_range(2)` first, which asks the kernel to copy from one
file to the other without the bytes ever reaching cat's memory. Lesson
1.2.2 gets you the plain `read`/`write` version.

**Opening a directory.** `open("somedir", O_RDONLY)` succeeds, because
directories are files you're allowed to open (that's how `ls` lists them,
via `getdents64`). Calling `read` on one fails with `EISDIR`. GNU cat
checks the type with `fstat` after opening, which avoids a race: a path
can change between a `stat` and an `open`, and a file descriptor can't.

**Why `std::filesystem` took an `error_code`.** Without one,
`is_directory` throws `filesystem_error` when it can't look, for instance
when a parent folder has no permission. With one, it just returns false
and you report the open error afterwards.

**`useless use of cat`.** `cat file | grep x` costs an extra process and
an extra copy through a pipe. `grep x file` or `grep x < file` is better.
It's an old Unix joke, and cat is still the right tool for actually
concatenating.
