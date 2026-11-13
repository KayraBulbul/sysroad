In `parse`, keep the `Command` being built and the `Step` being built.
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
The fork loop is 6.1.7's `run_pipeline` with one change: the child
calls `exec_command(sh, p.commands[i])` instead of `execvp`. Keep
`prev_read` across iterations, close the parent's copies at once, and
wait for every PID at the end, keeping the last status.
