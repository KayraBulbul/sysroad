#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <csignal>
#include <ostream>

// Sets handler as sig's action with sigaction: these sa_flags and an
// empty sa_mask. Returns the action it replaced. Throws std::system_error
// if sigaction fails.
struct sigaction install_handler(int sig, void (*handler)(int), int flags);

// The last signal stop_handler caught; 0 if none.
extern volatile sig_atomic_t stop_signal;

// Records sig in stop_signal, and does nothing else.
void stop_handler(int sig);

// Writes "tick N\n" to out every interval_ms milliseconds, N counting
// from 1, until stop_signal is set. Checks it before every tick. Returns
// the number of ticks.
int tick_until_stopped(int interval_ms, std::ostream& out);
