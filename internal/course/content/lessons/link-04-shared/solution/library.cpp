#include "library.hpp"

#include <dlfcn.h>

#include <stdexcept>
#include <utility>

Library::Library(const std::string& path) {
    handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (handle_ == nullptr) {
        throw std::runtime_error(dlerror());
    }
}

Library::~Library() {
    if (handle_ != nullptr) {
        dlclose(handle_);
    }
}

Library::Library(Library&& other) noexcept : handle_(std::exchange(other.handle_, nullptr)) {}

Library& Library::operator=(Library&& other) noexcept {
    if (this != &other) {
        if (handle_ != nullptr) {
            dlclose(handle_);
        }
        handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
}

void* Library::symbol(const std::string& name) const {
    if (handle_ == nullptr) {
        throw std::runtime_error("symbol(\"" + name + "\") on a closed Library");
    }
    dlerror();  // clear any old error
    void* p = dlsym(handle_, name.c_str());
    if (const char* err = dlerror()) {
        throw std::runtime_error(err);
    }
    return p;
}
