#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <memory>
#include <vector>

class IntList {
  public:
    IntList() = default;
    ~IntList();

    // One owner per node: no copies. (Moving it comes in the next chapter.)
    IntList(const IntList&) = delete;
    IntList& operator=(const IntList&) = delete;

    void push_front(int v);
    void push_back(int v);
    // Removes and returns the first value. The list must not be empty.
    int pop_front();

    bool contains(int v) const;
    std::vector<int> to_vector() const;
    // Reverses the order in place.
    void reverse();

    std::size_t size() const {
        return size_;
    }
    bool empty() const {
        return size_ == 0;
    }

  private:
    struct Node {
        int value;
        std::unique_ptr<Node> next;
    };
    std::unique_ptr<Node> head_;
    Node* tail_ = nullptr;  // the last node (owned by the one before it), or nullptr
    std::size_t size_ = 0;
};
