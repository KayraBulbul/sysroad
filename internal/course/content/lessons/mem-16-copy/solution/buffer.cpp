#include "buffer.hpp"

Buffer::~Buffer() {
    delete[] data_;
}

Buffer::Buffer(const Buffer& other) : data_(new char[other.size_]), size_(other.size_) {
    std::memcpy(data_, other.data_, size_);
}

Buffer& Buffer::operator=(const Buffer& other) {
    if (this == &other) {
        return *this;
    }
    char* fresh = new char[other.size_];
    std::memcpy(fresh, other.data_, other.size_);
    delete[] data_;
    data_ = fresh;
    size_ = other.size_;
    return *this;
}
