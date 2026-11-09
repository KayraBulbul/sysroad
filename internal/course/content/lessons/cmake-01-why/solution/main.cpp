// minimake [GOAL] < Buildfile: print the commands that would bring GOAL (or
// the first rule's target) up to date. You won't need to change this file.
//
// A Buildfile lists when each file last changed, then the rules:
//
//   time main.cpp 10
//   time main.o 20
//   app: main.o
//     c++ main.o -o app
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "minimake.hpp"

int main(int argc, char** argv) {
    Rules rules;
    Mtimes mtimes;
    std::string first;
    Rule* last = nullptr;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (line[0] == ' ' || line[0] == '\t') {  // the command of the rule above
            if (last != nullptr) {
                last->command = line.substr(line.find_first_not_of(" \t"));
            }
            continue;
        }
        std::istringstream in(line);
        std::string word;
        in >> word;
        if (word == "time") {
            std::string name;
            long t = 0;
            in >> name >> t;
            mtimes[name] = t;
            continue;
        }
        if (word.empty() || word.back() != ':') {
            std::cerr << "minimake: can't read: " << line << '\n';
            return 2;
        }
        Rule r;
        r.target = word.substr(0, word.size() - 1);
        while (in >> word) {
            r.deps.push_back(word);
        }
        if (first.empty()) {
            first = r.target;
        }
        last = &(rules[r.target] = r);
    }
    std::string goal = argc > 1 ? argv[1] : first;
    try {
        auto commands = plan(rules, mtimes, goal);
        if (commands.empty()) {
            std::cout << "minimake: '" << goal << "' is up to date.\n";
        }
        for (const auto& c : commands) {
            std::cout << c << '\n';
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "minimake: " << e.what() << '\n';
        return 2;
    }
}
