// Value categories, then how many copies each function makes. You won't need to change this file.
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "moves.hpp"

static const char* kind(Tracked&) {
    return "lvalue";
}
static const char* kind(const Tracked&) {
    return "const lvalue";
}
static const char* kind(Tracked&&) {
    return "rvalue";
}

static Tracked make() {
    return Tracked("made");
}

static void report(const char* what) {
    std::printf("%-22s %d cop%s, %d move%s\n", what, Tracked::copies,
                Tracked::copies == 1 ? "y" : "ies", Tracked::moves, Tracked::moves == 1 ? "" : "s");
    Tracked::reset();
}

int main() {
    Tracked t("t");
    const Tracked ct("ct");
    std::printf("t              is an %s\n", kind(t));
    std::printf("ct             is a %s\n", kind(ct));
    std::printf("Tracked(\"x\")   is an %s\n", kind(Tracked("x")));
    std::printf("make()         is an %s\n", kind(make()));
    std::printf("std::move(t)   is an %s\n\n", kind(std::move(t)));

    std::vector<Tracked> v;
    v.reserve(16);
    Tracked::reset();
    append(v, Tracked("a long value, so copying it allocates"));
    report("append:");
    Tracked last = take_last(v);
    report("take_last:");
    std::vector<Tracked> a(3, Tracked("x")), b(3, Tracked("y"));
    a.reserve(6);
    Tracked::reset();
    std::vector<Tracked> ab = concat(std::move(a), std::move(b));
    report("concat:");
    Tracked x("1"), y("2"), z("3");
    Tracked::reset();
    rotate3(x, y, z);
    report("rotate3:");
    const Tracked& l = longest(ab);
    report("longest:");
    std::printf("\nrotate3 gave %s %s %s, longest is \"%s\"\n", x.value.c_str(), y.value.c_str(),
                z.value.c_str(), l.value.c_str());
}
