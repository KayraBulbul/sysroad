#include "library.hpp"

#include <dlfcn.h>

#include <stdexcept>
#include <utility>

Library::Library(const std::string& path) {
    // TODO: dlopen, and throw if it fails.
    (void)path;
}

Library::~Library() {
    // TODO
}

Library::Library(Library&& other) noexcept {
    // TODO
    (void)other;
}

Library& Library::operator=(Library&& other) noexcept {
    // TODO
    (void)other;
    return *this;
}

void* Library::symbol(const std::string& name) const {
    // TODO: dlsym, and throw if it fails.
    (void)name;
    return nullptr;
}
