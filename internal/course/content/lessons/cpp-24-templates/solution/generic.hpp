#pragma once
// Templates live in headers: this is the file you write.

#include <concepts>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

// The largest element (the first one on a tie), or nullopt when v is empty.
template <std::totally_ordered T> std::optional<T> max_of(const std::vector<T>& v) {
    if (v.empty()) {
        return std::nullopt;
    }
    const T* best = &v[0];
    for (const T& x : v) {
        if (*best < x) {
            best = &x;
        }
    }
    return *best;
}

// A last-in, first-out stack of T.
template <typename T> class Stack {
  public:
    void push(const T& value) {
        items_.push_back(value);
    }

    // Removes and returns the top, or nullopt when the stack is empty.
    std::optional<T> pop() {
        if (items_.empty()) {
            return std::nullopt;
        }
        T value = std::move(items_.back());
        items_.pop_back();
        return value;
    }

    // The top element. Only call it when the stack isn't empty.
    const T& top() const {
        return items_.back();
    }

    std::size_t size() const {
        return items_.size();
    }

    bool empty() const {
        return items_.empty();
    }

  private:
    std::vector<T> items_;
};

// How many elements in [first, last) make pred return true.
template <typename It, typename Pred> std::size_t count_where(It first, It last, Pred pred) {
    std::size_t n = 0;
    for (; first != last; ++first) {
        if (pred(*first)) {
            ++n;
        }
    }
    return n;
}
