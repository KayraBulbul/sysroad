#include "bitset.hpp"

#include <bit>
#include <stdexcept>

static constexpr std::size_t kWordBits = 64;

static std::uint64_t mask_of(std::size_t i) {
    return std::uint64_t{1} << (i % kWordBits);
}

static void check(std::size_t i, std::size_t size) {
    if (i >= size) {
        throw std::out_of_range("Bitset: bit " + std::to_string(i) + " of " + std::to_string(size));
    }
}

// Clear the bits of the last word that are past the end.
static void trim(std::vector<std::uint64_t>& words, std::size_t size) {
    if (size % kWordBits != 0) {
        words.back() &= mask_of(size) - 1;
    }
}

// The lowest set bit at index `from` or above, or size.
static std::size_t find_from(const std::vector<std::uint64_t>& words, std::size_t size,
                             std::size_t from) {
    if (from >= size) {
        return size;
    }
    std::size_t w = from / kWordBits;
    std::uint64_t word = words[w] & (~std::uint64_t{0} << (from % kWordBits));
    while (true) {
        if (word != 0) {
            return w * kWordBits + static_cast<std::size_t>(std::countr_zero(word));
        }
        if (++w == words.size()) {
            return size;
        }
        word = words[w];
    }
}

Bitset::Bitset(std::size_t size) : size_(size), words_((size + kWordBits - 1) / kWordBits, 0) {}

bool Bitset::test(std::size_t i) const {
    check(i, size_);
    return (words_[i / kWordBits] & mask_of(i)) != 0;
}

void Bitset::set(std::size_t i, bool value) {
    check(i, size_);
    if (value) {
        words_[i / kWordBits] |= mask_of(i);
    } else {
        words_[i / kWordBits] &= ~mask_of(i);
    }
}

void Bitset::flip(std::size_t i) {
    check(i, size_);
    words_[i / kWordBits] ^= mask_of(i);
}

void Bitset::set_all() {
    for (std::uint64_t& word : words_) {
        word = ~std::uint64_t{0};
    }
    trim(words_, size_);
}

std::size_t Bitset::count() const {
    std::size_t n = 0;
    for (std::uint64_t word : words_) {
        n += static_cast<std::size_t>(std::popcount(word));
    }
    return n;
}

bool Bitset::any() const {
    for (std::uint64_t word : words_) {
        if (word != 0) {
            return true;
        }
    }
    return false;
}

std::size_t Bitset::find_first() const {
    return find_from(words_, size_, 0);
}

std::size_t Bitset::find_next(std::size_t i) const {
    return i >= size_ ? size_ : find_from(words_, size_, i + 1);
}

static void same_size(const Bitset& a, const Bitset& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Bitset: sizes differ");
    }
}

Bitset& Bitset::operator&=(const Bitset& other) {
    same_size(*this, other);
    for (std::size_t w = 0; w < words_.size(); ++w) {
        words_[w] &= other.words_[w];
    }
    return *this;
}

Bitset& Bitset::operator|=(const Bitset& other) {
    same_size(*this, other);
    for (std::size_t w = 0; w < words_.size(); ++w) {
        words_[w] |= other.words_[w];
    }
    return *this;
}

Bitset& Bitset::operator^=(const Bitset& other) {
    same_size(*this, other);
    for (std::size_t w = 0; w < words_.size(); ++w) {
        words_[w] ^= other.words_[w];
    }
    return *this;
}

Bitset Bitset::operator~() const {
    Bitset out = *this;
    for (std::uint64_t& word : out.words_) {
        word = ~word;
    }
    trim(out.words_, out.size_);
    return out;
}

std::string Bitset::to_string() const {
    std::string out;
    out.reserve(size_);
    for (std::size_t i = 0; i < size_; ++i) {
        out += (words_[i / kWordBits] & mask_of(i)) != 0 ? '1' : '0';
    }
    return out;
}
