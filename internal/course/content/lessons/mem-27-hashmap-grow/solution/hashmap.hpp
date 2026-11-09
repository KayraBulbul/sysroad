#pragma once
// Your HashMap from lesson 2.4.4, now with erase, rehashing and iteration.

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
        if (find(key) != nullptr) {
            return false;
        }
        if ((used_ + 1) * 4 > slots_.size() * 3) {
            // Mostly live entries: double. Mostly tombstones: same size, cleaned out.
            rehash((size_ + 1) * 2 > slots_.size() ? slots_.size() * 2 : slots_.size());
        }
        std::size_t mask = slots_.size() - 1;
        std::size_t i = hash_(key) & mask;
        while (slots_[i].state == State::Full) {
            i = (i + 1) & mask;
        }
        if (slots_[i].state == State::Empty) {
            ++used_;
        }
        slots_[i] = Slot{State::Full, key, std::move(value)};
        ++size_;
        return true;
    }

    // Removes key. Returns whether it was there.
    bool erase(const K& key) {
        std::size_t i = slot_for(key);
        if (slots_[i].state != State::Full) {
            return false;
        }
        slots_[i] = Slot{State::Deleted, K{}, V{}};  // a tombstone; frees the old key and value
        --size_;
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

    // Rebuilds the table with at least `capacity` slots (more if the entries
    // need them), leaving no tombstones behind.
    void rehash(std::size_t capacity) {
        std::size_t n = round_up_pow2(capacity);
        while ((size_ + 1) * 4 > n * 3) {
            n *= 2;
        }
        std::vector<Slot> old = std::move(slots_);
        slots_ = std::vector<Slot>(n);
        size_ = 0;
        used_ = 0;
        for (Slot& s : old) {
            if (s.state == State::Full) {
                insert(s.key, std::move(s.value));
            }
        }
    }

    // Calls f(key, value) for every entry, in table order.
    template <typename F> void for_each(F f) const {
        for (const Slot& s : slots_) {
            if (s.state == State::Full) {
                f(s.key, s.value);
            }
        }
    }

  private:
    enum class State : unsigned char { Empty, Full, Deleted };
    struct Slot {
        State state = State::Empty;
        K key{};
        V value{};
    };

    // The slot holding key, or else the empty slot where the search ended.
    // Tombstones don't stop the search.
    std::size_t slot_for(const K& key) const {
        std::size_t mask = slots_.size() - 1;
        std::size_t i = hash_(key) & mask;
        while (slots_[i].state != State::Empty &&
               !(slots_[i].state == State::Full && slots_[i].key == key)) {
            i = (i + 1) & mask;
        }
        return i;
    }

    static std::size_t round_up_pow2(std::size_t n) {
        std::size_t p = 1;
        while (p < n) {
            p *= 2;
        }
        return p;
    }

    std::vector<Slot> slots_;
    std::size_t size_ = 0;  // Full slots
    std::size_t used_ = 0;  // Full and Deleted slots: what probing has to walk over
    Hash hash_;
};
