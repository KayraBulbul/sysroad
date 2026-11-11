// Builds two plugins, loads them with your Library, and runs each on the
// text you give it. You won't need to change this file.
#include <dlfcn.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <vector>

#include "library.hpp"
#include "plugins.hpp"

static void a_function_in_this_program() {}

int main(int argc, char** argv) {
    std::string text = argc > 1 ? argv[1] : "hello, world";

    // Which file is each of these functions in? dladdr asks the loader.
    struct {
        const char* name;
        void* address;
    } where[] = {{"your code", reinterpret_cast<void*>(&a_function_in_this_program)},
                 {"rand", reinterpret_cast<void*>(&rand)},
                 {"std::terminate", reinterpret_cast<void*>(&std::terminate)}};
    for (const auto& w : where) {
        Dl_info info{};
        if (dladdr(w.address, &info) != 0) {
            std::printf("%-15s is in %s\n", w.name, info.dli_fname);
        }
    }
    std::printf("\n");

    try {
        std::vector<Library> plugins;
        plugins.emplace_back(build_plugin("shout", shout_source));
        plugins.emplace_back(build_plugin("reverse", reverse_source));
        for (const Library& lib : plugins) {
            auto name = lib.function<const char*()>("plugin_name");
            auto apply = lib.function<void(char*)>("plugin_apply");
            if (name == nullptr || apply == nullptr) {
                std::printf("symbol() gave a null pointer\n");
                return 1;
            }
            std::string s = text;
            apply(s.data());
            std::printf("%-8s %s\n", name(), s.c_str());
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
}
