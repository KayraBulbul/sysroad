// cat [-n] [FILE...]: copy the files (or standard input) to standard
// output. -n numbers every line. You won't need to change this file.
#include <format>
#include <iostream>
#include <string>

#include "coreutils/io.hpp"

int main(int argc, char** argv) {
    auto files = coreutils::args(argc, argv);
    bool number = false;
    if (!files.empty() && std::string_view(files[0]) == "-n") {
        number = true;
        files = files.subspan(1);
    }
    long line = 0;
    bool at_start = true;
    return coreutils::for_each_input("cat", files, [&](std::istream& in, std::string_view) {
        char c;
        while (in.get(c)) {
            if (number && at_start) {
                std::cout << std::format("{:>6}\t", ++line);
            }
            std::cout.put(c);
            at_start = c == '\n';
        }
    });
}
