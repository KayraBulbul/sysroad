In your parser, keep the command being built and the step being built.
A word or a redirection goes into the command. Any other operator first
finishes the command (it must not be empty), then:

- `|`: carry on with the same step
- `;`, `&&`, `||`: set the step's `next`, push the step, start a new one

At the end, push what's left. A trailing `;` is fine; a trailing `|`,
`&&` or `||` isn't.
---
```cpp
Step::Next before = Step::Then;     // the first step always runs
for (const Step& step : list) {
    bool run = before == Step::Then ||
               (before == Step::And && sh.last_status == 0) ||
               (before == Step::Or && sh.last_status != 0);
    // ...
    before = step.next;
}
```
---
The fork loop is 6.1.7's pipeline with one change: the child runs a
conch command (redirections, then a builtin or `execvp`) instead of
calling `execvp` straight away. The part of your lesson-3 code that runs
in the child is that, so make it a function both can call. Keep
`prev_read` across iterations, close the parent's copies at once, and
wait for every PID at the end, keeping the last status.
---
When `pipe` or `fork` fails, there may be children already running,
waiting on a pipe nobody will write to. Close what the parent still
holds (`prev_read`), `kill(pid, SIGKILL)` each one started, `waitpid`
each, then report. An exception carries the "which call failed" out of
the loop neatly:
`throw std::system_error(errno, std::generic_category(), "pipe");`.
