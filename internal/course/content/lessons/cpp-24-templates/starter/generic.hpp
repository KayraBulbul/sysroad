#pragma once
// Templates live in headers: this is the file you write.

#include <concepts>
#include <cstddef>
#include <optional>
#include <vector>

// The largest element (the first one on a tie), or nullopt when v is empty.
template <std::totally_ordered T> std::optional<T> max_of(const std::vector<T>& v) {
    return std::nullopt;  // TODO
}

// A last-in, first-out stack of T.
template <typename T> class Stack {
  public:
    void push(const T& value) {
        // TODO
    }

    // Removes and returns the top, or nullopt when the stack is empty.
    std::optional<T> pop() {
        return std::nullopt;  // TODO
    }

    // The top element. Only call it when the stack isn't empty.
    const T& top() const {
        return items_.back();
    }

    std::size_t size() const {
        return 0;  // TODO
    }

    bool empty() const {
        return true;  // TODO
    }

  private:
    std::vector<T> items_;
};

// How many elements in [first, last) make pred return true.
template <typename It, typename Pred> std::size_t count_where(It first, It last, Pred pred) {
    return 0;  // TODO
}
