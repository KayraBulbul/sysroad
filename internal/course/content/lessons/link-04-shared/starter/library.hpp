#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <string>

// A shared library loaded while the program runs, with dlopen. Closed
// again (dlclose) when the Library is destroyed. Move-only, like
// unique_ptr: two Library objects never own the same handle.
class Library {
  public:
    // Loads the library at path, resolving all its symbols now. Throws
    // std::runtime_error with dlerror()'s message if it can't.
    explicit Library(const std::string& path);
    ~Library();

    Library(Library&& other) noexcept;
    Library& operator=(Library&& other) noexcept;
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    // The address of the symbol called name. Throws std::runtime_error if
    // there's no such symbol, or this Library was moved from.
    void* symbol(const std::string& name) const;

    // The function called name, as a pointer of type F*. The function must
    // be extern "C" in the library, so its symbol is its plain name.
    template <typename F> F* function(const std::string& name) const {
        return reinterpret_cast<F*>(symbol(name));
    }

    // False once moved from.
    bool is_open() const {
        return handle_ != nullptr;
    }

  private:
    void* handle_ = nullptr;
};
