# sysroad

A terminal tracker for a systems-engineering learning roadmap: C → CSAPP → OSTEP →
shell, malloc, HTTP server, KV store with a WAL, interpreter, toy kernel → Zig.

- **Full-screen mode** to browse, edit the roadmap, write notes and log time
- **Sidebar mode** for a terminal split: the current step, what's next, recently done
- **Status line** for tmux
- **Streaks, timers, time logs**, and an ETA based on your 14-day pace
- **Syncs between machines** through a private git repo, offline-first

## Install

Requires Go 1.24+ and git (Mac: `brew install go`, Omarchy: `sudo pacman -S go`).

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

## Usage

```sh
sysroad                 # full screen
sysroad side            # sidebar
sysroad status          # one line: streak 4d · 1h20m · > Bomb Lab
sysroad log 45m [id]    # log time without opening the TUI (default: current step)
sysroad sync            # sync right now
```

Press `?` in full-screen mode for every key. The main ones:

| key | action |
|---|---|
| `space` / `p` / `x` | toggle done / in progress / skipped |
| `t` | start/stop a timer on the selected step (or section) |
| `L` | log time manually (`45m`, `1h30m`, `1.5h`) |
| `n` / `N` | edit the step's note / the section's note in `$EDITOR` |
| `enter` | step detail: full description, notes, time log |
| `a` `r` `e` `D` `J/K` | add, rename, edit as YAML, delete, reorder |
| `o` | open the step's link (or the section's first resource) |
| `c` | jump to the current step |

The sidebar supports `space`, `p`, `t`, `n`, `o`, and `j/k` to pick something other than
the current step.

"Current" means the step you most recently marked in progress. If none is in progress,
it's the first unfinished step on the main path (optional steps and the Side Quests
section are skipped).

### tmux

```tmux
# prefix + R opens the sidebar on the right
bind R split-window -h -l 44 "sysroad side"
# current step + streak in the status bar
set -g status-right '#(sysroad status -width 30) '
set -g status-interval 15
```

In Ghostty or Rex, open a native split and run `sysroad side`. Colours come from your
terminal's 16-colour palette, so sysroad follows your Omarchy theme.

## How sync works

The data repo contains:

- `roadmap.yaml`: sections and steps. You can edit it in the TUI or by hand.
- `events.jsonl`: an append-only log of status changes, notes and time entries.
  The current state is rebuilt by replaying it.
- `.gitattributes`: `events.jsonl merge=union`. If both machines added lines while
  offline, git keeps both sides, so there are never merge conflicts.
- `.local/`: machine-local state such as a running timer. Gitignored.

On startup the TUI fetches and rebases. After a change it commits and pushes once things
have been quiet for 15 s, pulls every 5 minutes, and syncs on quit. If you're offline,
changes stay committed locally and go out on the next sync. Two sysroad windows on the
same machine (e.g. sidebar + full screen) see each other's changes within a second.

The only possible conflict is editing the *same lines* of `roadmap.yaml` on both
machines before either syncs. In that case sync stops and asks you to resolve it in the
data repo by hand.

## Development

```sh
make test
SYSROAD_DIR=/tmp/sysroad-dev go run . init && SYSROAD_DIR=/tmp/sysroad-dev go run .
```

The default roadmap lives in `internal/roadmap/default.yaml` and is embedded in the
binary. It's copied into the data repo once, by `sysroad init`.
