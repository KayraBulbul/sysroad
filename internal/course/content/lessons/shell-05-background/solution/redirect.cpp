// The course's version of this file. You won't need to change it.
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <system_error>

#include "shell.hpp"

static void apply(const Redirect& r) {
    if (r.op == Redirect::Dup) {
        if (fcntl(r.from, F_GETFD) == -1) {
            throw std::system_error(EBADF, std::generic_category(), std::to_string(r.from));
        }
        if (r.from != r.fd && dup2(r.from, r.fd) == -1) {
            throw std::system_error(errno, std::generic_category(), std::to_string(r.fd));
        }
        return;
    }
    int flags = O_RDONLY;
    if (r.op == Redirect::Write) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (r.op == Redirect::Append) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    }
    int fd = open(r.path.c_str(), flags, 0666);
    if (fd == -1) {
        throw std::system_error(errno, std::generic_category(), r.path);
    }
    if (fd != r.fd) {
        int err = dup2(fd, r.fd) == -1 ? errno : 0;  // e.g. an fd above the limit
        close(fd);
        if (err != 0) {
            throw std::system_error(err, std::generic_category(), std::to_string(r.fd));
        }
    }
}

void apply_redirects(const std::vector<Redirect>& redirects) {
    for (const Redirect& r : redirects) {
        apply(r);
    }
}

SavedFds::SavedFds(const std::vector<Redirect>& redirects) {
    for (const Redirect& r : redirects) {
        bool seen = false;
        for (const auto& [fd, copy] : saved_) {
            seen = seen || fd == r.fd;
        }
        if (seen) {
            continue;
        }
        // A copy at 10 or above, out of the way, closed if we exec.
        int copy = fcntl(r.fd, F_DUPFD_CLOEXEC, 10);
        saved_.emplace_back(r.fd, copy);
    }
}

SavedFds::~SavedFds() {
    for (auto it = saved_.rbegin(); it != saved_.rend(); ++it) {
        if (it->second == -1) {
            close(it->first);  // it was closed before
        } else {
            dup2(it->second, it->first);
            close(it->second);
        }
    }
}
