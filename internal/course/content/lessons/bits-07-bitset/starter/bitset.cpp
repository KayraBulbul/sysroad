#include "bitset.hpp"

#include <bit>
#include <stdexcept>

Bitset::Bitset(std::size_t size) : size_(size) {
    // TODO: give words_ enough zeroed words to hold size bits.
}

bool Bitset::test(std::size_t i) const {
    // TODO
    (void)i;
    return false;
}

void Bitset::set(std::size_t i, bool value) {
    // TODO
    (void)i;
    (void)value;
}

void Bitset::flip(std::size_t i) {
    // TODO
    (void)i;
}

void Bitset::set_all() {
    // TODO: and keep the unused bits of the last word 0.
}

std::size_t Bitset::count() const {
    // TODO
    return 0;
}

bool Bitset::any() const {
    // TODO
    return false;
}

std::size_t Bitset::find_first() const {
    // TODO
    return size_;
}

std::size_t Bitset::find_next(std::size_t i) const {
    // TODO
    (void)i;
    return size_;
}

Bitset& Bitset::operator&=(const Bitset& other) {
    // TODO
    (void)other;
    return *this;
}

Bitset& Bitset::operator|=(const Bitset& other) {
    // TODO
    (void)other;
    return *this;
}

Bitset& Bitset::operator^=(const Bitset& other) {
    // TODO
    (void)other;
    return *this;
}

Bitset Bitset::operator~() const {
    // TODO
    return *this;
}

std::string Bitset::to_string() const {
    // TODO
    return "";
}
