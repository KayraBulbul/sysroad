#pragma once
// Your HashMap: open addressing with linear probing.

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

template <typename K, typename V, typename Hash = std::hash<K>> class HashMap {
  public:
    // capacity is rounded up to a power of two.
    explicit HashMap(std::size_t capacity = 8) : slots_(round_up_pow2(capacity)) {}

    std::size_t size() const noexcept {
        return size_;
    }
    std::size_t capacity() const noexcept {
        return slots_.size();
    }

    // Adds key -> value if key isn't there yet. Returns whether it was added.
    bool insert(const K& key, V value) {
        if ((size_ + 1) * 4 > slots_.size() * 3) {
            grow();
        }
        std::size_t i = slot_for(key);
        if (slots_[i].state == State::Full) {
            return false;
        }
        slots_[i] = Slot{State::Full, key, std::move(value)};
        ++size_;
        return true;
    }

    // The value stored for key, or nullptr.
    V* find(const K& key) {
        std::size_t i = slot_for(key);
        return slots_[i].state == State::Full ? &slots_[i].value : nullptr;
    }
    const V* find(const K& key) const {
        std::size_t i = slot_for(key);
        return slots_[i].state == State::Full ? &slots_[i].value : nullptr;
    }

    bool contains(const K& key) const {
        return find(key) != nullptr;
    }

    // The value for key, inserting V() first if it's missing.
    V& operator[](const K& key) {
        if (V* v = find(key)) {
            return *v;
        }
        insert(key, V());
        return *find(key);
    }

  private:
    enum class State : unsigned char { Empty, Full };
    struct Slot {
        State state = State::Empty;
        K key{};
        V value{};
    };

    // The slot holding key, or else the empty slot where it belongs.
    std::size_t slot_for(const K& key) const {
        std::size_t mask = slots_.size() - 1;
        std::size_t i = hash_(key) & mask;
        while (slots_[i].state == State::Full && !(slots_[i].key == key)) {
            i = (i + 1) & mask;
        }
        return i;
    }

    // Doubles the table and puts every entry back in.
    void grow() {
        std::vector<Slot> old = std::move(slots_);
        slots_ = std::vector<Slot>(old.size() * 2);
        size_ = 0;
        for (Slot& s : old) {
            if (s.state == State::Full) {
                insert(s.key, std::move(s.value));
            }
        }
    }

    static std::size_t round_up_pow2(std::size_t n) {
        std::size_t p = 1;
        while (p < n) {
            p *= 2;
        }
        return p;
    }

    std::vector<Slot> slots_;
    std::size_t size_ = 0;
    Hash hash_;
};
