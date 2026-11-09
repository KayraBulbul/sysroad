// The course's version of what you built in this chapter. You won't need to change this file.
#include "string.hpp"

#include <algorithm>
#include <cstring>
#include <string>

String::String(std::string_view s) {
    reserve(s.size());
    std::memcpy(data(), s.data(), s.size());
    size_ = s.size();
    data()[size_] = '\0';
}

String::~String() {
    delete[] heap_;
}

String::String(const String& other) : String(std::string_view(other)) {}

String& String::operator=(const String& other) {
    if (this == &other) {
        return *this;
    }
    if (other.size_ > cap_) {
        char* fresh = new char[other.size_ + 1];
        delete[] heap_;
        heap_ = fresh;
        cap_ = other.size_;
    }
    std::memcpy(data(), other.data(), other.size_ + 1);
    size_ = other.size_;
    return *this;
}

String::String(String&& other) noexcept {
    if (other.heap_ != nullptr) {
        heap_ = other.heap_;
        cap_ = other.cap_;
    } else {
        std::memcpy(buf_, other.buf_, sizeof buf_);
    }
    size_ = other.size_;
    other.heap_ = nullptr;
    other.size_ = 0;
    other.cap_ = inline_capacity;
    other.buf_[0] = '\0';
}

String& String::operator=(String&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    delete[] heap_;
    heap_ = nullptr;
    if (other.heap_ != nullptr) {
        heap_ = other.heap_;
        cap_ = other.cap_;
    } else {
        std::memcpy(buf_, other.buf_, sizeof buf_);
        cap_ = inline_capacity;
    }
    size_ = other.size_;
    other.heap_ = nullptr;
    other.size_ = 0;
    other.cap_ = inline_capacity;
    other.buf_[0] = '\0';
    return *this;
}

void String::reserve(std::size_t n) {
    if (n <= cap_) {
        return;
    }
    char* fresh = new char[n + 1];
    std::memcpy(fresh, data(), size_ + 1);
    delete[] heap_;
    heap_ = fresh;
    cap_ = n;
}

void String::push_back(char c) {
    if (size_ == cap_) {
        reserve(2 * cap_);
    }
    data()[size_++] = c;
    data()[size_] = '\0';
}

void String::append(std::string_view s) {
    std::size_t need = size_ + s.size();
    if (need > cap_) {
        // Build the new buffer before freeing the old one: s may point into it.
        std::size_t cap = std::max(need, 2 * cap_);
        char* fresh = new char[cap + 1];
        std::memcpy(fresh, data(), size_);
        std::memcpy(fresh + size_, s.data(), s.size());
        delete[] heap_;
        heap_ = fresh;
        cap_ = cap;
    } else {
        std::memmove(data() + size_, s.data(), s.size());
    }
    size_ = need;
    data()[size_] = '\0';
}

void String::clear() noexcept {
    size_ = 0;
    data()[0] = '\0';
}
