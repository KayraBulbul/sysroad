// layout TYPE...: where a struct with these fields puts them. You won't need to change this file.
#include <cstdio>
#include <iostream>
#include <map>
#include <string>

#include "layout.hpp"

static const char* bytes(std::size_t n) {
    return n == 1 ? "byte" : "bytes";
}

// TYPE or TYPE[N].
static bool parse_type(const std::string& arg, Field& f) {
    static const std::map<std::string, std::pair<std::size_t, std::size_t>> types = {
        {"char", {1, 1}},  {"bool", {1, 1}},  {"short", {2, 2}},  {"int", {4, 4}},
        {"float", {4, 4}}, {"long", {8, 8}},  {"double", {8, 8}}, {"ptr", {8, 8}},
        {"int16", {2, 2}}, {"int32", {4, 4}}, {"int64", {8, 8}},
    };
    std::string base = arg;
    std::size_t count = 1;
    if (auto lb = arg.find('['); lb != std::string::npos && arg.back() == ']') {
        base = arg.substr(0, lb);
        try {
            count = std::stoul(arg.substr(lb + 1, arg.size() - lb - 2));
        } catch (const std::exception&) {
            return false;
        }
    }
    auto it = types.find(base);
    if (it == types.end() || count == 0) {
        return false;
    }
    f = {arg, it->second.first * count, it->second.second};
    return true;
}

int main(int argc, char** argv) {
    std::vector<Field> fields;
    for (int i = 1; i < argc; ++i) {
        Field f;
        if (!parse_type(argv[i], f)) {
            std::cerr << "layout: unknown type " << argv[i]
                      << " (try char bool short int float long double ptr int16 int32 int64, or "
                         "int[3])\n";
            return 2;
        }
        fields.push_back(f);
    }
    if (fields.empty()) {
        std::cerr << "usage: layout TYPE...\n";
        return 2;
    }
    Layout l = lay_out(fields);
    std::printf("offset  size  field\n");
    std::size_t padding = l.tail_padding;
    for (const Placed& p : l.fields) {
        if (p.padding_before > 0) {
            std::printf("%6zu        (%zu %s padding)\n", p.offset - p.padding_before,
                        p.padding_before, bytes(p.padding_before));
        }
        std::printf("%6zu %5zu  %s\n", p.offset, p.size, p.name.c_str());
        padding += p.padding_before;
    }
    if (l.tail_padding > 0) {
        std::printf("%6zu        (%zu %s padding)\n", l.size - l.tail_padding, l.tail_padding,
                    bytes(l.tail_padding));
    }
    std::printf("sizeof %zu, alignof %zu, %zu %s of padding\n", l.size, l.align, padding,
                bytes(padding));
}
