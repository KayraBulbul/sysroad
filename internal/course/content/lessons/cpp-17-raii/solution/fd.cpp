#include "fd.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

Fd::~Fd() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

int Fd::release() {
    int fd = fd_;
    fd_ = -1;
    return fd;
}

void Fd::reset(int fd) {
    if (fd_ >= 0 && fd_ != fd) {
        ::close(fd_);
    }
    fd_ = fd;
}

Fd open_read(const char* path) {
    return Fd(::open(path, O_RDONLY));
}

std::string read_all(const Fd& fd) {
    std::string out;
    char buf[4096];
    for (;;) {
        ssize_t n = ::read(fd.get(), buf, sizeof buf);
        if (n > 0) {
            out.append(buf, static_cast<std::size_t>(n));
        } else if (n == 0 || errno != EINTR) {
            break;  // end of file, or a real error
        }
    }
    return out;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: fdcat FILE\n";
        return 2;
    }
    Fd fd = open_read(argv[1]);
    if (!fd.valid()) {
        std::cerr << "fdcat: " << argv[1] << ": " << std::strerror(errno) << '\n';
        return 1;
    }
    std::cout << read_all(fd);
    return 0;
}
