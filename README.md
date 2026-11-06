# sysroad

A boot.dev-style systems programming course in C++, in your terminal. Short lessons
with a task, Run and Submit backed by real tests, hints, a solution to peek at, and an
**Under the hood** section that unlocks once you pass. The course runs from your own
coreutils through a text editor, shell, malloc, HTTP server, KV store with a WAL and an
interpreter, to xv6 kernel labs.

- **Lessons** built with clang++ (C++20, AddressSanitizer + UBSan) and tested with
  [doctest](https://github.com/doctest/doctest) and program-output checks
- **Sidebar mode** for a terminal split next to your editor: the lesson on top, the
  output of your last Run/Submit underneath, even ones you start from the editor
- **Full-screen mode** to browse the course, with the lesson and its output side by side
- **XP, levels, chapter progress dots, streaks, timers** and an ETA from your 14-day pace
- **Syncs your progress and your code** between machines through a private git repo

## Install

Requires Go 1.24+, git and clang (Mac: `brew install go` and `xcode-select --install`;
Omarchy: `sudo pacman -S go clang gdb`).

```sh
go install github.com/KayraBulbul/sysroad@latest
# or from a checkout:
git clone https://github.com/KayraBulbul/sysroad ~/dev/sysroad && cd ~/dev/sysroad && make install
```

Both install to `$(go env GOPATH)/bin`, so make sure that's on your PATH.

## First-time setup

On the **first machine**, create the data repo (default location `~/.local/share/sysroad`,
override with `$SYSROAD_DIR`) and push it to a private GitHub repo:

```sh
sysroad init
gh repo create sysroad-data --private --source ~/.local/share/sysroad --push
```

On **every other machine**:

```sh
sysroad init -clone https://github.com/<you>/sysroad-data.git
```

Upgrading from the old C roadmap is automatic: on first open, `roadmap.yaml` moves to
`archive/roadmap-c.yaml`, and the steps that still apply carry their progress over.
Update sysroad on every machine, because older versions can't read the new layout.

## Doing a lesson

Put your editor in one split and `sysroad side` in the other. Every lesson has a folder
of starter code in the data repo:

```sh
cd "$(sysroad path)"     # the current lesson's folder
sysroad edit             # or open its file in $EDITOR directly
```

| sidebar key | shell command | |
|---|---|---|
| `r` | `sysroad run` | build, run the visible tests, stop at the first failure |
| `s` | `sysroad submit` | run every test, including hidden ones; a pass completes the lesson |
| `h` | `sysroad hint` | reveal the next hint |
| `u` | `sysroad lesson` | Under the hood, once you've passed |
| `v` | `sysroad solution` | peek at the solution (the sidebar asks first) |
| `R` | `sysroad reset` | put the starter code back; your files move to `.local/reset-backup/` |
| `e` | `sysroad edit` | open the lesson's file in `$EDITOR` |

The shell commands act on the lesson whose folder you're in, else the current one, or
take a lesson ID. `sysroad run` exits non-zero on failure, so it works as an editor
keymap; the sidebar picks up the result either way. In the sidebar `j/k` scrolls the
lesson, `J/K` scrolls the output, `[`/`]` browse lessons, `c` goes back to the current
one and `S` syncs.

"Current" means the step you most recently started. Running a lesson starts it. If
nothing is in progress, it's the first unfinished step on the main path. Optional steps,
side quests and lessons that aren't written yet are skipped.

## Full screen

Press `?` in full-screen mode for every key. The main ones:

| key | action |
|---|---|
| `enter` | open a lesson (or step) |
| `space` / `p` / `x` | toggle done / in progress / skipped (lessons complete by Submit) |
| `t` / `L` | start/stop a timer on the selected step / log time manually |
| `n` / `N` | edit the step's note / the section's note in `$EDITOR` |
| `a` `r` `e` `D` `J/K` | add, rename, edit as YAML, delete, reorder your own sections |
| `o` | open the step's link |
| `c` | jump to the current step |

Course chapters are read-only. Press `a` in the left pane to add a section of your own;
your sections live in `custom.yaml` and come after the course.

### tmux and theming

```tmux
# prefix + R opens the sidebar on the right
bind R split-window -h -l 52 "sysroad side"
# current step, level and streak in the status bar
set -g status-right '#(sysroad status -width 30) '
set -g status-interval 15
```

In Ghostty, open a native split and run `sysroad side`. Colours come from your
terminal's 16-colour palette, so sysroad follows your Omarchy theme live. On a Mac,
set Ghostty and Neovim to a theme you like. [`extras/mac-vigil`](extras/mac-vigil)
has Vigil.

## How sync works

The data repo contains:

- `events.jsonl`: an append-only log of status changes, notes and time entries.
  The current state is rebuilt by replaying it.
- `work/<lesson-id>/`: your code for each lesson.
- `custom.yaml`: sections and steps of your own.
- `.gitattributes`: `events.jsonl merge=union`. If both machines added lines while
  offline, git keeps both sides, so there are never merge conflicts in the log.
- `.local/`: machine-local state such as a running timer, build output, the latest
  Run/Submit results and revealed hints. Gitignored, as are `*.o`, `a.out` and `build/`.

On startup the TUI fetches and rebases. After a change it commits and pushes once things
have been quiet for 15 s, pulls every 5 minutes, and syncs on quit. A passing
`sysroad submit` syncs straight away. If you're offline, changes stay committed locally
and go out on the next sync. Two sysroad windows on the same machine see each other's
changes within a second.

The only possible conflict is editing the *same lines* of the same lesson file (or
`custom.yaml`) on both machines before either syncs. In that case sync stops and asks you
to resolve it in the data repo by hand.

## Development

```sh
make test     # includes building every lesson's starter and solution with clang++
SYSROAD_DIR=/tmp/sysroad-dev go run . init && SYSROAD_DIR=/tmp/sysroad-dev go run .
```

The course lives in `internal/course/content/` and is embedded in the binary:

- `course.yaml`: phases → chapters → lessons. A lesson without a folder shows as "soon".
- `lessons/<id>/lesson.md`: front matter (`title`, `minutes`, `xp`, `files` you edit,
  an optional `run:` sample, `deeper:` links) and the brief, ending in `## Assignment`.
- `starter/` and `solution/`: the files. `make test` checks that every starter fails Run
  and every solution passes Submit.
- `tests/visible_*.cpp` (Run and Submit) and `tests/hidden_*.cpp` (Submit only): doctest
  test cases. If your file has a `main`, it's renamed so doctest's runs instead.
- `checks.yaml`: program-level tests (args, stdin, files → expected stdout and exit code);
  `hidden: true` ones run on Submit only.
- `hints.md` (split on `---`) and `under_the_hood.md`.
