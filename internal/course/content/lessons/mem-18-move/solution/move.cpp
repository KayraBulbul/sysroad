#include "fd.hpp"

#include <utility>

Fd::Fd(Fd&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

Fd& Fd::operator=(Fd&& other) noexcept {
    if (this != &other) {
        reset(other.release());
    }
    return *this;
}

std::vector<Fd> open_all(const std::vector<std::string>& paths, std::vector<std::string>& failed) {
    std::vector<Fd> out;
    for (const std::string& p : paths) {
        Fd fd = open_read(p.c_str());
        if (fd.valid()) {
            out.push_back(std::move(fd));
        } else {
            failed.push_back(p);
        }
    }
    return out;
}

Fd open_first(const std::vector<std::string>& paths) {
    for (const std::string& p : paths) {
        Fd fd = open_read(p.c_str());
        if (fd.valid()) {
            return fd;
        }
    }
    return Fd();
}
