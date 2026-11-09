// Copies some buffers. You won't need to change this file.
#include <cstdio>
#include <string>

#include "buffer.hpp"

static void show(const char* label, const Buffer& b, const char* note = "") {
    std::printf("%-7s \"%s\"%s\n", label, std::string(b.view()).c_str(), note);
}

int main() {
    Buffer a(std::string_view("hello"));
    Buffer b = a;  // copy constructor
    b[0] = 'j';
    show("a:", a);
    show("b:", b, "   (a copy of a, then b[0] = 'j')");
    std::printf("%s\n\n", a.data() == b.data() ? "a and b share their bytes!"
                                               : "a and b have their own bytes");

    Buffer c(std::string_view("a longer buffer"));
    c = a;  // copy assignment
    show("c = a:", c);
    Buffer& same = c;
    c = same;  // assigning an object to itself must work too
    show("c = c:", c);
}
