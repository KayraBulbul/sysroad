#include <cstdio>
#include <fstream>
#include <iostream>

#include "shell.hpp"

static const size_t kHistoryMax = 1000;

void add_history(Shell& sh, const std::string& line) {
    if (line.find_first_not_of(" \t") == std::string::npos) {
        return;
    }
    if (!sh.history.empty() && sh.history.back() == line) {
        return;
    }
    sh.history.push_back(line);
    if (sh.history.size() > kHistoryMax) {
        sh.history.erase(sh.history.begin());
    }
}

int builtin_history(Shell& sh, const std::vector<std::string>& argv) {
    size_t from = 0;
    if (argv.size() > 1) {
        size_t n = std::stoul(argv[1]);
        from = n < sh.history.size() ? sh.history.size() - n : 0;
    }
    for (size_t i = from; i < sh.history.size(); ++i) {
        char num[16];
        std::snprintf(num, sizeof num, "%5zu", i + 1);
        std::cout << num << "  " << sh.history[i] << '\n';
    }
    return 0;
}

void load_history(Shell& sh, const std::string& path) {
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        add_history(sh, line);
    }
}

void save_history(const Shell& sh, const std::string& path) {
    std::string tmp = path + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        for (const auto& line : sh.history) {
            out << line << '\n';
        }
        if (!out) {
            return;
        }
    }
    std::rename(tmp.c_str(), path.c_str());
}
