#include "config.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

ConfigError::ConfigError(int line, const std::string& msg)
    : std::runtime_error("line " + std::to_string(line) + ": " + msg), line_(line) {}

static std::string_view strip(std::string_view s) {
    auto b = s.find_first_not_of(" \t\r");
    if (b == std::string_view::npos) {
        return {};
    }
    auto e = s.find_last_not_of(" \t\r");
    return s.substr(b, e - b + 1);
}

std::map<std::string, std::string> parse_config(std::istream& in) {
    std::map<std::string, std::string> cfg;
    std::string line;
    int n = 0;
    while (std::getline(in, line)) {
        ++n;
        std::string_view text = strip(line);
        if (text.empty() || text.starts_with('#')) {
            continue;
        }
        auto eq = text.find('=');
        if (eq == std::string_view::npos) {
            throw ConfigError(n, "expected key = value");
        }
        std::string key(strip(text.substr(0, eq)));
        std::string value(strip(text.substr(eq + 1)));
        if (key.empty()) {
            throw ConfigError(n, "empty key");
        }
        if (cfg.contains(key)) {
            throw ConfigError(n, "duplicate key '" + key + "'");
        }
        cfg[key] = value;
    }
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
    try {
        for (const auto& [key, value] : parse_config(f)) {
            std::cout << key << '=' << value << '\n';
        }
    } catch (const ConfigError& e) {
        std::cerr << "config: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
