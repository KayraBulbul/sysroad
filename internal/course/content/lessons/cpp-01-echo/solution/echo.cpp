#include <iostream>
#include <string>
#include <vector>

// Joins args with a single space between each one.
//   {"hello", "world"}  ->  "hello world"
//   {}                  ->  ""
std::string join_args(const std::vector<std::string>& args) {
    std::string out;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) {
            out += ' ';
        }
        out += args[i];
    }
    return out;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    bool newline = true;
    if (!args.empty() && args[0] == "-n") {
        newline = false;
        args.erase(args.begin());
    }
    std::cout << join_args(args);
    if (newline) {
        std::cout << '\n';
    }
    return 0;
}
