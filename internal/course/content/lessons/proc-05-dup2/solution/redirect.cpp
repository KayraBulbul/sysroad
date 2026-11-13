#include "redirect.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

static std::optional<int> number(const std::string& s) {
    if (s.empty() || s.size() > 4 || s.find_first_not_of("0123456789") != std::string::npos) {
        return std::nullopt;
    }
    return std::stoi(s);
}

std::optional<Redirect> parse_redirect(const std::string& word) {
    size_t op = word.find_first_of("<>");
    if (op == std::string::npos) {
        return std::nullopt;
    }
    Redirect r;
    std::string target;
    if (word.compare(op, 2, ">>") == 0) {
        r.fd = 1;
        r.op = Redirect::Append;
        target = word.substr(op + 2);
    } else if (word.compare(op, 2, ">&") == 0 || word.compare(op, 2, "<&") == 0) {
        r.fd = word[op] == '<' ? 0 : 1;
        r.op = Redirect::Dup;
        auto from = number(word.substr(op + 2));
        if (!from) {
            return std::nullopt;
        }
        r.from = *from;
    } else {
        r.fd = word[op] == '<' ? 0 : 1;
        r.op = word[op] == '<' ? Redirect::Read : Redirect::Write;
        target = word.substr(op + 1);
    }
    if (op > 0) {
        auto fd = number(word.substr(0, op));
        if (!fd) {
            return std::nullopt;
        }
        r.fd = *fd;
    }
    if (r.op != Redirect::Dup) {
        if (target.empty() || target.find_first_of("<>") != std::string::npos) {
            return std::nullopt;
        }
        r.path = target;
    }
    return r;
}

void apply_redirect(const Redirect& r) {
    if (r.op == Redirect::Dup) {
        if (r.from != r.fd && dup2(r.from, r.fd) == -1) {
            throw std::system_error(errno, std::generic_category(), std::to_string(r.from));
        }
        if (r.from == r.fd && fcntl(r.fd, F_GETFD) == -1) {
            throw std::system_error(EBADF, std::generic_category(), std::to_string(r.from));
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

Status run_redirected(const std::vector<std::string>& argv,
                      const std::vector<Redirect>& redirects) {
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
        try {
            for (const auto& r : redirects) {
                apply_redirect(r);
            }
        } catch (const std::system_error& e) {
            std::fprintf(stderr, "redir: %s\n", e.what());
            _exit(1);
        }
        execvp(args[0], args.data());
        std::fprintf(stderr, "redir: %s: %s\n", args[0], std::strerror(errno));
        _exit(127);
    }
    return wait_for(pid);
}

// redir [REDIRECT]... COMMAND [ARG]...: runs COMMAND with its fds
// rearranged, e.g. redir "<in.txt" ">out.txt" "2>&1" sort
int main(int argc, char* argv[]) {
    std::vector<Redirect> redirects;
    int i = 1;
    for (; i < argc; ++i) {
        auto r = parse_redirect(argv[i]);
        if (!r) {
            break;
        }
        redirects.push_back(*r);
    }
    if (i == argc) {
        std::cerr << "usage: redir [REDIRECT]... COMMAND [ARG]...\n";
        return 2;
    }
    Status s = run_redirected(std::vector<std::string>(argv + i, argv + argc), redirects);
    return shell_status(s);
}
