#include "fd.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

Fd::~Fd() {
    // TODO
}

int Fd::release() {
    return -1;  // TODO
}

void Fd::reset(int fd) {
    // TODO
}

Fd open_read(const char* path) {
    return Fd();  // TODO
}

std::string read_all(const Fd& fd) {
    return "";  // TODO
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
