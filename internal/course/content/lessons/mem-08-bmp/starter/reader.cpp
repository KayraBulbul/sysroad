// From lesson 2.1.6. You won't need to change this file.
#include "reader.hpp"

#include "endian.hpp"

std::span<const unsigned char> ByteReader::bytes(std::size_t n) {
    if (n > remaining()) {
        ok_ = false;
        return {};
    }
    auto out = data_.subspan(pos_, n);
    pos_ += n;
    return out;
}

std::uint8_t ByteReader::u8() {
    auto b = bytes(1);
    return b.empty() ? 0 : b[0];
}

std::uint16_t ByteReader::le16() {
    auto b = bytes(2);
    return b.empty() ? 0 : load_le16(b.data());
}

std::uint32_t ByteReader::le32() {
    auto b = bytes(4);
    return b.empty() ? 0 : load_le32(b.data());
}

std::uint16_t ByteReader::be16() {
    auto b = bytes(2);
    return b.empty() ? 0 : load_be16(b.data());
}

std::uint32_t ByteReader::be32() {
    auto b = bytes(4);
    return b.empty() ? 0 : load_be32(b.data());
}

void ByteReader::skip(std::size_t n) {
    bytes(n);
}

void ByteReader::seek(std::size_t pos) {
    if (pos > data_.size()) {
        ok_ = false;
        return;
    }
    pos_ = pos;
}
