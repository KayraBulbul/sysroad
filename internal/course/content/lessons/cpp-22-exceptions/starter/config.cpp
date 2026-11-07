#include "config.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

ConfigError::ConfigError(int line, const std::string& msg)
    : std::runtime_error(msg), line_(line) {}  // TODO: what() should start with "line N: "

std::map<std::string, std::string> parse_config(std::istream& in) {
    std::map<std::string, std::string> cfg;
    // TODO: read lines, skip blanks and comments, split at '=', throw on errors
    return cfg;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: config FILE\n";
        return 2;
    }
    std::ifstream f(argv[1]);
    if (!f) {
        std::cerr << "config: can't open " << argv[1] << '\n';
        return 1;
    }
    // TODO: catch ConfigError, print "config: <what>" and return 1
    for (const auto& [key, value] : parse_config(f)) {
        std::cout << key << '=' << value << '\n';
    }
    return 0;
}
