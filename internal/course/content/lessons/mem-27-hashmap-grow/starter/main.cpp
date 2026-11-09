// A tiny key-value shell on standard input:
//   set KEY VALUE   get KEY   del KEY   list
// You won't need to change this file.
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "hashmap.hpp"

int main() {
    HashMap<std::string, std::string> map;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string cmd, key, value;
        in >> cmd >> key;
        std::getline(in >> std::ws, value);
        if (cmd == "set") {
            map[key] = value;
        } else if (cmd == "get") {
            if (const std::string* v = map.find(key)) {
                std::cout << key << " = " << *v << '\n';
            } else {
                std::cout << key << " is not set\n";
            }
        } else if (cmd == "del") {
            std::cout << (map.erase(key) ? "deleted " : "no such key: ") << key << '\n';
        } else if (cmd == "list") {
            std::vector<std::pair<std::string, std::string>> all;
            map.for_each(
                [&](const std::string& k, const std::string& v) { all.emplace_back(k, v); });
            std::sort(all.begin(), all.end());
            for (const auto& [k, v] : all) {
                std::cout << "  " << k << " = " << v << '\n';
            }
            std::cout << map.size() << (map.size() == 1 ? " entry\n" : " entries\n");
        } else if (!cmd.empty()) {
            std::cout << "unknown command: " << cmd << '\n';
        }
    }
}
