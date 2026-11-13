#pragma once

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>

inline int lowest_free_fd() {
    int fd = ::open("/dev/null", O_RDONLY);
    ::close(fd);
    return fd;
}

// Runs f in a child that SIGALRM kills after `seconds`. Returns f's
// result, or -1 if it hung or crashed.
template <class F> int within(unsigned seconds, F f) {
    std::fflush(nullptr);
    pid_t pid = ::fork();
    if (pid == 0) {
        ::signal(SIGALRM, SIG_DFL);
        ::alarm(seconds);
        int r = f();
        ::_exit(r);
    }
    int st = 0;
    ::waitpid(pid, &st, 0);
    return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}
