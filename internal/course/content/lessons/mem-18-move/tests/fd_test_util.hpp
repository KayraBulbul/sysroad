#pragma once

#include <fcntl.h>
#include <fstream>
#include <string>

inline bool is_open(int fd) {
    return fd >= 0 && ::fcntl(fd, F_GETFD) != -1;
}

inline void write_file(const std::string& name, const std::string& data) {
    std::ofstream(name, std::ios::binary) << data;
}
