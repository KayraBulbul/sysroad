#include "redirect.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <system_error>

std::optional<Redirect> parse_redirect(const std::string& word) {
    (void)word;  // TODO
    return std::nullopt;
}

void apply_redirect(const Redirect& r) {
    (void)r;  // TODO: open (or not, for Dup), dup2, close
}

Status run_redirected(const std::vector<std::string>& argv,
                      const std::vector<Redirect>& redirects) {
    // TODO: fork; in the child apply each redirect, then execvp.
    (void)redirects;
    return wait_for(start(argv));
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
