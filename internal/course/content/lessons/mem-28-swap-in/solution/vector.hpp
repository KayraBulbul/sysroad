#pragma once
// The course's version of what you built in this chapter. You won't need to change this file.

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <new>
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
        clear();
        ::operator delete(data_);
    }

    Vector(const Vector& other) {
        reserve(other.size_);
        for (std::size_t i = 0; i < other.size_; ++i) {
            new (data_ + i) T(other.data_[i]);
            ++size_;
        }
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Vector(Vector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)),
          cap_(std::exchange(other.cap_, 0)) {}

    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            Vector tmp(std::move(other));
            swap(tmp);
        }
        return *this;
    }

    // Room for at least n elements. Never shrinks.
    void reserve(std::size_t n) {
        if (n <= cap_) {
            return;
        }
        T* fresh = static_cast<T*>(::operator new(n * sizeof(T)));
        relocate_to(fresh);
        cap_ = n;
    }

    // Constructs a T from args at the end. Returns it.
    template <typename... Args> T& emplace_back(Args&&... args) {
        if (size_ < cap_) {
            T* p = new (data_ + size_) T(std::forward<Args>(args)...);
            ++size_;
            return *p;
        }
        // Full: build the new element in the new buffer first, while args
        // (which may refer to one of our elements) are still valid.
        std::size_t cap = cap_ == 0 ? 1 : 2 * cap_;
        T* fresh = static_cast<T*>(::operator new(cap * sizeof(T)));
        T* p = new (fresh + size_) T(std::forward<Args>(args)...);
        relocate_to(fresh);
        cap_ = cap;
        ++size_;
        return *p;
    }

    void push_back(const T& value) {
        emplace_back(value);
    }
    void push_back(T&& value) {
        emplace_back(std::move(value));
    }

    // Destroys the last element.
    void pop_back() {
        data_[--size_].~T();
    }

    // Destroys every element. Keeps the capacity.
    void clear() noexcept {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }

    // Grows with value-initialised elements (T()), or destroys from the end.
    void resize(std::size_t n) {
        while (size_ > n) {
            pop_back();
        }
        reserve(n);
        while (size_ < n) {
            emplace_back();
        }
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

    // Iterators. For a vector they're just pointers.
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept {
        return data_;
    }
    iterator end() noexcept {
        return data_ + size_;
    }
    const_iterator begin() const noexcept {
        return data_;
    }
    const_iterator end() const noexcept {
        return data_ + size_;
    }

    // Inserts value before pos, shifting the rest up. Returns an iterator to
    // the inserted element.
    iterator insert(const_iterator pos, T value) {
        std::size_t i = pos - begin();  // an index survives the growth below; pos doesn't
        emplace_back(std::move(value));
        std::rotate(begin() + i, end() - 1, end());
        return begin() + i;
    }

    // Removes the element at pos, shifting the rest down. Returns an iterator
    // to the element after it (or end()).
    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }

    // Removes [first, last). Returns an iterator to the element after them.
    iterator erase(const_iterator first, const_iterator last) {
        std::size_t i = first - begin();
        std::size_t n = last - first;
        std::move(begin() + i + n, end(), begin() + i);
        for (std::size_t k = 0; k < n; ++k) {
            pop_back();
        }
        return begin() + i;
    }

  private:
    // Moves the elements into fresh, destroys the originals, frees the old buffer.
    void relocate_to(T* fresh) {
        for (std::size_t i = 0; i < size_; ++i) {
            new (fresh + i) T(std::move_if_noexcept(data_[i]));
            data_[i].~T();
        }
        ::operator delete(data_);
        data_ = fresh;
    }

    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t cap_ = 0;
};

// Removes every element for which pred(element) is true, keeping the order
// of the rest. Returns how many were removed.
template <typename T, typename Pred> std::size_t erase_if(Vector<T>& v, Pred pred) {
    auto keep_end = std::remove_if(v.begin(), v.end(), pred);
    std::size_t removed = v.end() - keep_end;
    v.erase(keep_end, v.end());
    return removed;
}
