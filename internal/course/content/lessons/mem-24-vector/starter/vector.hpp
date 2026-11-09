#pragma once
// Your Vector<T>. Everything lives in this header because it's a template.

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <utility>

template <typename T> class Vector {
  public:
    Vector() noexcept = default;

    Vector(std::initializer_list<T> init) {
        reserve(init.size());
        for (const T& v : init) {
            push_back(v);
        }
    }

    ~Vector() {
        // TODO: destroy the elements, free the buffer.
    }

    Vector(const Vector& other) {
        // TODO
        (void)other;
    }

    Vector& operator=(const Vector& other) {
        // TODO
        (void)other;
        return *this;
    }

    Vector(Vector&& other) noexcept {
        // TODO: steal; leave other empty.
        (void)other;
    }

    Vector& operator=(Vector&& other) noexcept {
        // TODO
        (void)other;
        return *this;
    }

    // Room for at least n elements. Never shrinks.
    void reserve(std::size_t n) {
        // TODO: raw buffer, move the elements across, destroy the old ones, free.
        (void)n;
    }

    // Constructs a T from args at the end. Returns it.
    template <typename... Args> T& emplace_back(Args&&... args) {
        // TODO: grow when full; careful, args may refer to an element of this Vector.
        ((void)args, ...);
        throw std::logic_error("Vector::emplace_back isn't written yet");
    }

    void push_back(const T& value) {
        emplace_back(value);
    }
    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    // Destroys the last element.
    void pop_back() {
        // TODO
    }

    // Destroys every element. Keeps the capacity.
    void clear() noexcept {
        // TODO
    }

    // Grows with value-initialised elements (T()), or destroys from the end.
    void resize(std::size_t n) {
        // TODO
        (void)n;
    }

    void swap(Vector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(cap_, other.cap_);
    }

    std::size_t size() const noexcept {
        return size_;
    }
    std::size_t capacity() const noexcept {
        return cap_;
    }
    bool empty() const noexcept {
        return size_ == 0;
    }
    T& operator[](std::size_t i) {
        return data_[i];
    }
    const T& operator[](std::size_t i) const {
        return data_[i];
    }
    T& back() {
        return data_[size_ - 1];
    }
    const T& back() const {
        return data_[size_ - 1];
    }
    T* data() noexcept {
        return data_;
    }
    const T* data() const noexcept {
        return data_;
    }

  private:
    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t cap_ = 0;
};
