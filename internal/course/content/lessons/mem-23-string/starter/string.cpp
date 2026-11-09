#include "string.hpp"

#include <algorithm>
#include <cstring>
#include <string>

String::String(std::string_view s) {
    // TODO: reserve room, copy s in, terminate.
    (void)s;
}

String::~String() {
    // TODO
}

String::String(const String& other) {
    // TODO
    (void)other;
}

String& String::operator=(const String& other) {
    // TODO
    (void)other;
    return *this;
}

String::String(String&& other) noexcept {
    // TODO: steal the heap buffer, or copy the inline text; leave other empty.
    (void)other;
}

String& String::operator=(String&& other) noexcept {
    // TODO
    (void)other;
    return *this;
}

void String::reserve(std::size_t n) {
    // TODO
    (void)n;
}

void String::push_back(char c) {
    // TODO
    (void)c;
}

void String::append(std::string_view s) {
    // TODO: careful, s might point into this String.
    (void)s;
}

void String::clear() noexcept {
    // TODO
}
