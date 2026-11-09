#include "buffer.hpp"

Buffer::~Buffer() {
    // TODO: free the bytes.
}

// What the compiler would have written: it copies the pointer, not the bytes.
Buffer::Buffer(const Buffer& other) : data_(other.data_), size_(other.size_) {}

Buffer& Buffer::operator=(const Buffer& other) {
    data_ = other.data_;
    size_ = other.size_;
    return *this;
}
