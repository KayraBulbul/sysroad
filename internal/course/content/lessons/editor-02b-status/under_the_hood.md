**Why two bars?** vi and kilo put the file's name and position in one
line and messages in another, because a message that covers the status
for five seconds would hide what you're editing. Emacs calls them the
mode line and the echo area, and lets the echo area grow to show a
long message.

**Five seconds, checked when drawing.** kite only redraws after a key,
so a message can stay up longer than five seconds if you don't press
anything: nobody draws the frame without it. Timers would fix it, with
`poll` on the terminal and a timeout, which is how editors that redraw
on their own (a clock, a spinner) wait for keys and time at once.

**Inverse video** is the oldest way to stand out. The VT100 could do
bold, underline, blink and inverse, and no colour at all, so
`\x1b[7m` works on every terminal there is. Swapping the colours also
means the bar follows your theme, light or dark.

**`std::time` and clocks.** `std::time` is wall-clock seconds. If the
system clock jumps (an NTP correction, a laptop waking up), a message
can vanish early or late. For measuring intervals, `std::chrono::
steady_clock` never goes backwards; for a status message, nobody
minds.
