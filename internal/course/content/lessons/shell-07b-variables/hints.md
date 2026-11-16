Marking an assignment in the lexer, once a word's text is read: count
the name characters at its start (letters, digits, `_`); it's an
assignment if there's at least one, the first isn't a digit, and the
next character is `=`. A quote anywhere in the name stops the count
before the `=`, so `'A'=1` isn't one.
---
A `std::map<std::string, std::string>` in your `Shell` holds the shell
variables. Four small functions keep the rules in one place: look a
variable up (shell first, then `getenv`), set one (`setenv` if it's in
the environment already, else the map), export one (move it from the
map to the environment, or `setenv` it to `""`), and unset one (both).
---
For a builtin's temporary assignments, save each variable as it was
(in the map? in the environment?) before setting it with `setenv`, and
put it back after the builtin, newest first. A small struct with a
destructor does the putting back on every way out.
---
To keep `cd`'s own `PWD`: have the builtins note which variables they
set (a list in your `Shell`, cleared before each builtin), and skip
those when putting variables back.
