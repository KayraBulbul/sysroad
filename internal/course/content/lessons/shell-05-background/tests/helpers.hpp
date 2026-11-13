#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

// Points stdout and stderr at files for as long as it lives.
struct Captured {
    int saved1, saved2;
    Captured() {
        std::fflush(nullptr);
        saved1 = dup(1);
        saved2 = dup(2);
        int o = open("cap-out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        int e = open("cap-err.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(o, 1);
        dup2(e, 2);
        close(o);
        close(e);
    }
    ~Captured() {
        std::fflush(nullptr);
        dup2(saved1, 1);
        dup2(saved2, 2);
        close(saved1);
        close(saved2);
    }
    static std::string read(const char* path) {
        std::ifstream in(path);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
    static std::string out() {
        return read("cap-out.txt");
    }
    static std::string err() {
        return read("cap-err.txt");
    }
};
