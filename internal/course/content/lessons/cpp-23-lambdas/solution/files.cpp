#include "files.hpp"

#include <algorithm>
#include <filesystem>
#include <format>
#include <iostream>
#include <numeric>

void sort_by_size(std::vector<Entry>& v) {
    std::sort(v.begin(), v.end(), [](const Entry& a, const Entry& b) {
        if (a.size != b.size) {
            return a.size > b.size;
        }
        return a.name < b.name;
    });
}

std::uint64_t total_size(const std::vector<Entry>& v) {
    return std::accumulate(v.begin(), v.end(), std::uint64_t{0},
                           [](std::uint64_t sum, const Entry& e) { return sum + e.size; });
}

std::size_t count_at_least(const std::vector<Entry>& v, std::uint64_t min) {
    return static_cast<std::size_t>(
        std::count_if(v.begin(), v.end(), [min](const Entry& e) { return e.size >= min; }));
}

std::optional<Entry> largest(const std::vector<Entry>& v) {
    auto it = std::max_element(v.begin(), v.end(),
                               [](const Entry& a, const Entry& b) { return a.size < b.size; });
    if (it == v.end()) {
        return std::nullopt;
    }
    return *it;
}

void remove_empty(std::vector<Entry>& v) {
    std::erase_if(v, [](const Entry& e) { return e.size == 0; });
}

std::vector<std::string> names_with_ext(const std::vector<Entry>& v, std::string_view ext) {
    std::vector<std::string> out;
    for (const Entry& e : v) {
        if (e.name.ends_with(ext)) {
            out.push_back(e.name);
        }
    }
    return out;
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
