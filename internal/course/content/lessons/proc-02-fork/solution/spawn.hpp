#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <sys/types.h>
#include <sys/wait.h>

#include <cerrno>
#include <functional>
#include <vector>

// Forks. The child runs body and exits with the number it returns, or
// with 1 if it throws. The parent gets the child's PID straight away.
// Throws std::system_error if fork fails.
pid_t spawn(const std::function<int()>& body);

// Spawns n children at once, child i running body(i), then waits for
// all of them. Returns their exit codes, in order.
std::vector<int> run_all(int n, const std::function<int(int)>& body);

// Waits for the child to finish and returns its exit code. (Lesson 4
// takes this apart.)
inline int wait_exit_code(pid_t pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}
