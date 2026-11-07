// The drill. Don't change this file: investigate it with the debugger, and
// put what you find in answers.hpp.
#include "drill.hpp"

#include <cstdio>

std::uint32_t step(std::uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

std::uint32_t walk(std::uint32_t seed, int n) {
    for (int i = 0; i < n; ++i) {
        seed = step(seed);
    }
    return seed;
}

std::uint64_t checksum(int n) {
    std::uint64_t acc = 0;
    std::uint32_t x = 2026;
    for (int i = 0; i < n; ++i) {
        x = step(x);
        acc = acc * 31 + x % 1000;
    }
    return acc % 1000000007;
}

std::string make_name(std::uint32_t seed) {
    std::string name;
    for (int i = 0; i < 6; ++i) {
        seed = step(seed);
        name += static_cast<char>('a' + seed % 26);
    }
    return name;
}

struct User {
    const char* name;
    int level;
};

static const User users[] = {{"alice", 3}, {"bob", 5}, {"carol", 8}};

static const User* find_user(std::string_view name) {
    for (const User& u : users) {
        if (name == u.name) {
            return &u;
        }
    }
    return nullptr;
}

int level_of(std::string_view name) {
    const User* u = find_user(name);
    return u->level;
}

int main() {
    std::printf("walk(1, 10) = %u\n", walk(1, 10));
    std::printf("checksum(100) = %llu\n", static_cast<unsigned long long>(checksum(100)));
    std::string team[] = {"alice", "carol", make_name(walk(7, 3)), "bob"};
    for (const std::string& name : team) {
        std::printf("%s is level %d\n", name.c_str(), level_of(name));
    }
    return 0;
}
