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
    std::vector<Command> commands;
    if (words.empty()) {
        return commands;
    }
    commands.emplace_back();
    for (const auto& w : words) {
        if (w != "|") {
            commands.back().push_back(w);
            continue;
        }
        if (commands.back().empty()) {
            throw std::invalid_argument("empty command before |");
        }
        commands.emplace_back();
    }
    if (commands.back().empty()) {
        throw std::invalid_argument("empty command after |");
    }
    return commands;
}

static std::vector<char*> c_args(const Command& command) {
    std::vector<char*> args;
    for (const auto& a : command) {
        args.push_back(const_cast<char*>(a.c_str()));
    }
    args.push_back(nullptr);
    return args;
}

std::vector<Status> run_pipeline(const std::vector<Command>& commands) {
    std::vector<pid_t> pids;
    int prev_read = -1;  // the read end of the pipe from the last command
    int error = 0;
    for (size_t i = 0; i < commands.size(); ++i) {
        std::vector<char*> args = c_args(commands[i]);
        int p[2] = {-1, -1};
        if (i + 1 < commands.size() && pipe(p) == -1) {
            error = errno;
            break;
        }
        std::fflush(nullptr);
        pid_t pid = fork();
        if (pid == 0) {
            if (prev_read != -1) {
                dup2(prev_read, 0);
                close(prev_read);
            }
            if (p[1] != -1) {
                dup2(p[1], 1);
                close(p[1]);
                close(p[0]);
            }
            signal(SIGPIPE, SIG_DFL);
            execvp(args[0], args.data());
            std::fprintf(stderr, "%s: %s\n", args[0], std::strerror(errno));
            _exit(127);
        }
        // The parent's copies: the child has its own now.
        if (prev_read != -1) {
            close(prev_read);
        }
        if (p[1] != -1) {
            close(p[1]);
        }
        prev_read = p[0];
        if (pid == -1) {
            error = errno;
            break;
        }
        pids.push_back(pid);
    }
    if (prev_read != -1) {
        close(prev_read);
    }
    std::vector<Status> statuses;
    for (pid_t pid : pids) {
        statuses.push_back(wait_for(pid));
    }
    if (error != 0) {
        throw std::system_error(error, std::generic_category(), "run_pipeline");
    }
    return statuses;
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
