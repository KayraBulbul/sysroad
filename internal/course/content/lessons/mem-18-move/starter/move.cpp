#include "fd.hpp"

#include <utility>

Fd::Fd(Fd&& other) noexcept {
    // TODO: take other's fd, and leave other with -1.
    (void)other;
}

Fd& Fd::operator=(Fd&& other) noexcept {
    // TODO: close what this one has, take other's. Careful with a = std::move(a).
    (void)other;
    return *this;
}

std::vector<Fd> open_all(const std::vector<std::string>& paths, std::vector<std::string>& failed) {
    // TODO
    (void)paths;
    (void)failed;
    return {};
}

Fd open_first(const std::vector<std::string>& paths) {
    // TODO
    (void)paths;
    return Fd();
}
