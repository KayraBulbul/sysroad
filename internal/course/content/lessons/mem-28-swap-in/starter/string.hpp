#pragma once
// The course's version of what you built in this chapter. You won't need to change this file.

#include <compare>
#include <cstddef>
#include <functional>
#include <string_view>

// A growable string that owns its chars. Up to 15 chars live inside the
// object itself; longer text goes on the heap. The text is always followed
// by a '\0', so c_str() works.
class String {
  public:
    static constexpr std::size_t inline_capacity = 15;

    String() noexcept = default;  // empty and inline
    explicit String(std::string_view s);
    ~String();

    String(const String& other);             // deep copy
    String& operator=(const String& other);  // deep copy; s = s is fine
    String(String&& other) noexcept;         // steals; other is left empty
    String& operator=(String&& other) noexcept;

    // At least n chars of room, without counting the '\0'.
    void reserve(std::size_t n);
    void push_back(char c);
    void append(std::string_view s);  // s may point into this String
    String& operator+=(std::string_view s) {
        append(s);
        return *this;
    }
    // Empties the string, keeping its capacity.
    void clear() noexcept;

    std::size_t size() const noexcept {
        return size_;
    }
    std::size_t capacity() const noexcept {
        return cap_;
    }
    bool empty() const noexcept {
        return size_ == 0;
    }
    bool is_inline() const noexcept {
        return heap_ == nullptr;
    }
    char* data() noexcept {
        return heap_ != nullptr ? heap_ : buf_;
    }
    const char* data() const noexcept {
        return heap_ != nullptr ? heap_ : buf_;
    }
    const char* c_str() const noexcept {
        return data();
    }
    char& operator[](std::size_t i) {
        return data()[i];
    }
    char operator[](std::size_t i) const {
        return data()[i];
    }
    char* begin() noexcept {
        return data();
    }
    char* end() noexcept {
        return data() + size_;
    }
    const char* begin() const noexcept {
        return data();
    }
    const char* end() const noexcept {
        return data() + size_;
    }
    operator std::string_view() const noexcept {
        return {data(), size_};
    }

    friend bool operator==(const String& a, const String& b) noexcept {
        return std::string_view(a) == std::string_view(b);
    }
    friend bool operator==(const String& a, std::string_view b) noexcept {
        return std::string_view(a) == b;
    }
    friend auto operator<=>(const String& a, const String& b) noexcept {
        return std::string_view(a) <=> std::string_view(b);
    }

  private:
    char* heap_ = nullptr;  // nullptr while the text is inline
    std::size_t size_ = 0;
    std::size_t cap_ = inline_capacity;
    char buf_[inline_capacity + 1] = {};
};

// So a String can be a key in hash tables.
template <> struct std::hash<String> {
    std::size_t operator()(const String& s) const noexcept {
        return std::hash<std::string_view>{}(s);
    }
};
