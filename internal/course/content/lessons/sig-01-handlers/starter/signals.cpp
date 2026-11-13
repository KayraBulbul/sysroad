#include "signals.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <system_error>

volatile sig_atomic_t stop_signal = 0;

struct sigaction install_handler(int sig, void (*handler)(int), int flags) {
    struct sigaction old{};
    // TODO: fill in a struct sigaction and install it.
    (void)sig;
    (void)handler;
    (void)flags;
    return old;
}

void stop_handler(int sig) {
    (void)sig;  // TODO
}

int tick_until_stopped(int interval_ms, std::ostream& out) {
    // TODO: sleep with usleep, print a tick, repeat, until stop_signal.
    (void)interval_ms;
    (void)out;
    return 0;
}

static const char* name(int sig) {
    switch (sig) {
    case SIGINT:
        return "SIGINT";
    case SIGTERM:
        return "SIGTERM";
    case SIGALRM:
        return "SIGALRM";
    default:
        return "a signal";
    }
}

// ticker [-t SECONDS]: ticks every 100 ms until Ctrl-C (SIGINT), SIGTERM,
// or with -t, the SIGALRM that alarm sends after SECONDS.
int main(int argc, char* argv[]) {
    for (int sig : {SIGINT, SIGTERM, SIGALRM}) {
        install_handler(sig, stop_handler, 0);
    }
    if (argc == 3 && std::string(argv[1]) == "-t") {
        alarm(std::atoi(argv[2]));
    }
    std::cout << "ticking as pid " << getpid() << "; Ctrl-C, or kill " << getpid() << ", stops me"
              << std::endl;
    int ticks = tick_until_stopped(100, std::cout);
    std::cout << "stopped by " << name(stop_signal) << " after " << ticks << " ticks\n";
    return 0;
}
