#include "files.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <iostream>
#include <numeric>

void sort_by_size(std::vector<Entry>& v) {
    // TODO: std::sort with a lambda
}

std::uint64_t total_size(const std::vector<Entry>& v) {
    return 0;  // TODO: std::accumulate
}

std::size_t count_at_least(const std::vector<Entry>& v, std::uint64_t min) {
    return 0;  // TODO: std::count_if with a capture
}

std::optional<Entry> largest(const std::vector<Entry>& v) {
    return std::nullopt;  // TODO: std::max_element
}

void remove_empty(std::vector<Entry>& v) {
    // TODO: std::erase_if
}

std::vector<std::string> names_with_ext(const std::vector<Entry>& v, std::string_view ext) {
    return {};  // TODO
}

// Lists the regular files in a folder, biggest first.
int main(int argc, char* argv[]) {
    namespace fs = std::filesystem;
    fs::path dir = argc > 1 ? argv[1] : ".";
    std::vector<Entry> v;
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(dir, ec)) {
        if (e.is_regular_file()) {
            v.push_back({e.path().filename().string(), e.file_size()});
        }
    }
    if (ec) {
        std::cerr << "files: " << dir.string() << ": " << ec.message() << '\n';
        return 1;
    }
    std::size_t files = v.size();
    remove_empty(v);
    sort_by_size(v);
    for (const Entry& e : v) {
        std::cout << std::format("{:>8}  {}\n", e.size, e.name);
    }
    std::cout << std::format("{} files, {} empty, {} bytes", files, files - v.size(),
                             total_size(v));
    if (auto big = largest(v)) {
        std::cout << ", largest " << big->name;
    }
    std::cout << '\n';
    return 0;
}
