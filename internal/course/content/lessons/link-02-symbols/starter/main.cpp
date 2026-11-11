// Prints a report of the numbers on the command line. You won't need to
// change this file.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "report.hpp"
#include "stats.hpp"

int main(int argc, char** argv) {
    std::vector<double> xs;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-q") == 0) {
            verbosity = 0;
        } else {
            xs.push_back(std::atof(argv[i]));
        }
    }
    std::printf("%s\n", report(xs).c_str());
}
