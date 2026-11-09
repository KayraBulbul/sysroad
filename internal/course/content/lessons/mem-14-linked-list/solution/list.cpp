#include "list.hpp"

#include <utility>

IntList::~IntList() {
    while (head_) {
        head_ = std::move(head_->next);
    }
}

void IntList::push_front(int v) {
    head_ = std::make_unique<Node>(Node{v, std::move(head_)});
    if (tail_ == nullptr) {
        tail_ = head_.get();
    }
    ++size_;
}

void IntList::push_back(int v) {
    auto n = std::make_unique<Node>(Node{v, nullptr});
    Node* raw = n.get();
    if (tail_ != nullptr) {
        tail_->next = std::move(n);
    } else {
        head_ = std::move(n);
    }
    tail_ = raw;
    ++size_;
}

int IntList::pop_front() {
    int v = head_->value;
    head_ = std::move(head_->next);
    if (!head_) {
        tail_ = nullptr;
    }
    --size_;
    return v;
}

bool IntList::contains(int v) const {
    for (const Node* n = head_.get(); n != nullptr; n = n->next.get()) {
        if (n->value == v) {
            return true;
        }
    }
    return false;
}

std::vector<int> IntList::to_vector() const {
    std::vector<int> out;
    out.reserve(size_);
    for (const Node* n = head_.get(); n != nullptr; n = n->next.get()) {
        out.push_back(n->value);
    }
    return out;
}

void IntList::reverse() {
    std::unique_ptr<Node> done;
    tail_ = head_.get();
    while (head_) {
        std::unique_ptr<Node> rest = std::move(head_->next);
        head_->next = std::move(done);
        done = std::move(head_);
        head_ = std::move(rest);
    }
    head_ = std::move(done);
}
