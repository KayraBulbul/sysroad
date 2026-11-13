#include <unistd.h>

#include <csignal>
#include <cstdio>

#include "shell.hpp"

int run_list(Shell& sh, const List& list) {
    // TODO: && and || decide whether the next pipeline runs.
    for (const Step& step : list) {
        sh.last_status = run_pipeline(sh, step.pipeline);
        if (sh.exiting) {
            break;
        }
    }
    return sh.last_status;
}

int run_pipeline(Shell& sh, const Pipeline& p) {
    // TODO: every command, connected by pipes. For now only the first runs.
    const Command& c = p.commands[0];
    if (c.argv.empty() || find_builtin(c.argv[0]) != nullptr) {
        return run_in_shell(sh, c);
    }
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGPIPE, SIG_DFL);
        exec_command(sh, c);
    }
    return shell_status(wait_for(pid));
}
