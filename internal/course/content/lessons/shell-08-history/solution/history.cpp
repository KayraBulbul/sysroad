#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
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
        const std::string& count = argv[1];
        if (count.empty() || count.size() > 9 ||
            count.find_first_not_of("0123456789") != std::string::npos) {
            std::cerr << "conch: history: " << count << ": numeric argument required\n";
            return 1;
        }
        size_t n = std::stoul(count);
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
    // History can hold passwords typed by mistake: only the owner may read
    // it. fchmod covers a .tmp left over from before with looser bits.
    int fd = open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
        return;
    }
    fchmod(fd, 0600);
    std::string all;
    for (const auto& line : sh.history) {
        all += line + '\n';
    }
    size_t done = 0;
    while (done < all.size()) {
        ssize_t n = write(fd, all.data() + done, all.size() - done);
        if (n == -1 && errno == EINTR) {
            continue;
        }
        if (n <= 0) {
            close(fd);
            return;  // keep the old history rather than half a new one
        }
        done += static_cast<size_t>(n);
    }
    if (close(fd) == -1) {
        return;
    }
    std::rename(tmp.c_str(), path.c_str());
}
