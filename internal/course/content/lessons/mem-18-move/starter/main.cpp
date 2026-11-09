// move FILE...: open every file first, then print them in order. You won't need to change this
// file.
#include <iostream>
#include <string>
#include <vector>

#include "fd.hpp"

int main(int argc, char** argv) {
    std::vector<std::string> paths(argv + 1, argv + argc);
    std::vector<std::string> failed;
    std::vector<Fd> files = open_all(paths, failed);
    for (const std::string& f : failed) {
        std::cerr << "move: " << f << ": can't open\n";
    }
    std::cout << "opened " << files.size() << " of " << paths.size() << '\n';
    for (const Fd& fd : files) {
        std::cout << read_all(fd);
    }
    return failed.empty() ? 0 : 1;
}
