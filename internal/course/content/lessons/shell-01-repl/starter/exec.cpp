#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "shell.hpp"

void run_line(Shell& sh, const std::string& line) {
    // TODO: split the line; run the words, unless there are none; set $?.
    (void)sh;
    (void)line;
}

int run_command(const std::vector<std::string>& argv) {
    // TODO: fork, execvp in the child (127 and a message if it isn't
    // found), wait in the parent.
    (void)argv;
    return 0;
}
