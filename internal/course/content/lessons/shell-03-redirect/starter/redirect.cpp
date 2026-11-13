#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <system_error>

#include "shell.hpp"

void apply_redirects(const std::vector<Redirect>& redirects) {
    (void)redirects;  // TODO: lesson 6.1.5, for each one in order
}

SavedFds::SavedFds(const std::vector<Redirect>& redirects) {
    (void)redirects;  // TODO: keep a copy of each fd they'll change
}

SavedFds::~SavedFds() {
    // TODO: put each one back
}
