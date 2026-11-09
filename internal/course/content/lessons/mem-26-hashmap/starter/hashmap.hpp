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
        // TODO: grow if the load would pass 3/4, then find the slot.
        (void)key;
        (void)value;
        return false;
    }

    // The value stored for key, or nullptr.
    V* find(const K& key) {
        // TODO
        (void)key;
        return nullptr;
    }
    const V* find(const K& key) const {
        // TODO
        (void)key;
        return nullptr;
    }

    bool contains(const K& key) const {
        return find(key) != nullptr;
    }

    // The value for key, inserting V() first if it's missing.
    V& operator[](const K& key) {
        // TODO
        (void)key;
        return slots_[0].value;
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
        // TODO: start at hash_(key) masked to the table, step on until you
        // reach the key or an empty slot.
        (void)key;
        return 0;
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
