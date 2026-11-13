#include "pipeline.hpp"

#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <system_error>

std::vector<Command> split_pipeline(const std::vector<std::string>& words) {
    (void)words;  // TODO
    return {};
}

std::vector<Status> run_pipeline(const std::vector<Command>& commands) {
    (void)commands;  // TODO
    return {};
}

// pipeline COMMAND [| COMMAND]...: quote the | so your shell doesn't
// take it, e.g. pipeline printf 'b\na\n' '|' sort
int main(int argc, char* argv[]) {
    std::vector<Command> commands;
    try {
        commands = split_pipeline(std::vector<std::string>(argv + 1, argv + argc));
    } catch (const std::invalid_argument& e) {
        std::cerr << "pipeline: " << e.what() << '\n';
        commands.clear();
    }
    if (commands.empty()) {
        std::cerr << "usage: pipeline COMMAND ['|' COMMAND]...\n";
        return 2;
    }
    std::vector<Status> statuses = run_pipeline(commands);
    if (statuses.size() != commands.size()) {
        std::cerr << "pipeline: got " << statuses.size() << " statuses for " << commands.size()
                  << " commands\n";
        return 1;
    }
    for (size_t i = 0; i < statuses.size(); ++i) {
        const Status& s = statuses[i];
        bool quiet = s.ok() || (s.kind == Status::Signaled && s.code == SIGPIPE);
        if (!quiet) {
            std::cerr << "pipeline: " << commands[i][0] << ": " << describe(s) << '\n';
        }
    }
    return shell_status(statuses.back());
}
