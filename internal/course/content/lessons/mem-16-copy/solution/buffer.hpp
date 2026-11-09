#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <cstring>
#include <string_view>

// A block of bytes on the heap, owned by one Buffer.
class Buffer {
  public:
    explicit Buffer(std::size_t size = 0) : data_(new char[size]()), size_(size) {}
    explicit Buffer(std::string_view text) : data_(new char[text.size()]), size_(text.size()) {
        std::memcpy(data_, text.data(), size_);
    }
    ~Buffer();

    Buffer(const Buffer& other);             // a deep copy
    Buffer& operator=(const Buffer& other);  // frees the old bytes, copies the new

    std::size_t size() const {
        return size_;
    }
    char& operator[](std::size_t i) {
        return data_[i];
    }
    std::string_view view() const {
        return {data_, size_};
    }
    const char* data() const {
        return data_;
    }

  private:
    char* data_;
    std::size_t size_;
};
