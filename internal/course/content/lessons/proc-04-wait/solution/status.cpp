#include "status.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <system_error>

pid_t start(const std::vector<std::string>& argv) {
    std::vector<char*> args;
    for (const auto& a : argv) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    std::fflush(nullptr);
    pid_t pid = fork();
    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "fork");
    }
    if (pid == 0) {
        execvp(args[0], args.data());
        std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
        _exit(127);
    }
    return pid;
}

Status decode(int wstatus) {
    Status s;
    if (WIFEXITED(wstatus)) {
        s.kind = Status::Exited;
        s.code = WEXITSTATUS(wstatus);
    } else if (WIFSIGNALED(wstatus)) {
        s.kind = Status::Signaled;
        s.code = WTERMSIG(wstatus);
        s.core_dumped = WCOREDUMP(wstatus);
    }
    return s;
}

Status wait_for(pid_t pid) {
    int wstatus = 0;
    while (waitpid(pid, &wstatus, 0) == -1) {
        if (errno != EINTR) {
            throw std::system_error(errno, std::generic_category(), "waitpid");
        }
    }
    return decode(wstatus);
}

std::string signal_name(int sig) {
    switch (sig) {
    case SIGHUP:
        return "SIGHUP";
    case SIGINT:
        return "SIGINT";
    case SIGQUIT:
        return "SIGQUIT";
    case SIGILL:
        return "SIGILL";
    case SIGTRAP:
        return "SIGTRAP";
    case SIGABRT:
        return "SIGABRT";
    case SIGBUS:
        return "SIGBUS";
    case SIGFPE:
        return "SIGFPE";
    case SIGKILL:
        return "SIGKILL";
    case SIGUSR1:
        return "SIGUSR1";
    case SIGSEGV:
        return "SIGSEGV";
    case SIGUSR2:
        return "SIGUSR2";
    case SIGPIPE:
        return "SIGPIPE";
    case SIGALRM:
        return "SIGALRM";
    case SIGTERM:
        return "SIGTERM";
    case SIGCHLD:
        return "SIGCHLD";
    case SIGCONT:
        return "SIGCONT";
    case SIGSTOP:
        return "SIGSTOP";
    case SIGTSTP:
        return "SIGTSTP";
    case SIGTTIN:
        return "SIGTTIN";
    case SIGTTOU:
        return "SIGTTOU";
    default:
        return "signal " + std::to_string(sig);
    }
}

std::string describe(const Status& s) {
    if (s.kind == Status::Exited) {
        return "exited " + std::to_string(s.code);
    }
    std::string out = "killed by " + signal_name(s.code);
    if (s.core_dumped) {
        out += " (core dumped)";
    }
    return out;
}

int shell_status(const Status& s) {
    return s.kind == Status::Exited ? s.code : 128 + s.code;
}

std::vector<std::pair<pid_t, Status>> reap_finished() {
    std::vector<std::pair<pid_t, Status>> done;
    for (;;) {
        int wstatus = 0;
        pid_t pid = waitpid(-1, &wstatus, WNOHANG);
        if (pid > 0) {
            done.emplace_back(pid, decode(wstatus));
        } else if (pid == -1 && errno == EINTR) {
            continue;
        } else {
            return done;  // 0: the rest are still running; -1 (ECHILD): no children
        }
    }
}

// retry [-n TIMES] COMMAND [ARG]...: runs COMMAND until it succeeds, at
// most TIMES times (3 by default), and exits the way its last run did.
int main(int argc, char* argv[]) {
    int times = 3;
    int first = 1;
    if (argc > 2 && std::string(argv[1]) == "-n") {
        times = std::atoi(argv[2]);
        first = 3;
    }
    if (first >= argc || times < 1) {
        std::cerr << "usage: retry [-n TIMES] COMMAND [ARG]...\n";
        return 2;
    }
    std::vector<std::string> command(argv + first, argv + argc);
    Status s;
    for (int attempt = 1; attempt <= times; ++attempt) {
        std::cout.flush();
        s = wait_for(start(command));
        std::cout << "retry: attempt " << attempt << " of " << times << ": " << describe(s) << '\n';
        if (s.ok()) {
            break;
        }
    }
    return shell_status(s);
}
