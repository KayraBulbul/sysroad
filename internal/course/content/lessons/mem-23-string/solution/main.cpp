// Builds some Strings and counts their allocations. You won't need to change this file.
#include <cstdio>
#include <utility>

#include "alloc_count.hpp"
#include "string.hpp"

static void show(const char* label, const String& s, const AllocScope& scope) {
    std::printf("%-26s size %4zu  capacity %4zu  %-6s  %ld alloc%s\n", label, s.size(),
                s.capacity(), s.is_inline() ? "inline" : "heap", scope.news(),
                scope.news() == 1 ? "" : "s");
}

int main() {
    std::printf("sizeof(String) = %zu\n\n", sizeof(String));
    {
        AllocScope a;
        String s("hello");
        show("String(\"hello\")", s, a);
    }
    {
        AllocScope a;
        String s("exactly fifteen");
        show("15 chars", s, a);
        s.push_back('!');
        show("...push_back('!')", s, a);
    }
    {
        AllocScope a;
        String s;
        for (int i = 0; i < 1000; ++i) {
            s.push_back('x');
        }
        show("1000 push_backs", s, a);
        String copy = s;
        show("a copy of that", copy, a);
        String moved = std::move(copy);
        show("moved from the copy", moved, a);
        std::printf("%-26s size %4zu  %s\n", "...what's left behind", copy.size(),
                    copy.is_inline() ? "inline" : "heap");
    }
    {
        String s("abcdefghij");
        s.append(s);
        s.append(s);
        std::printf("\nappend(s) twice: \"%s\"\n", s.c_str());
    }
}
