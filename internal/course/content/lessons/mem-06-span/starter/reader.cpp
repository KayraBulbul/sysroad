#include "reader.hpp"

#include "endian.hpp"

std::span<const unsigned char> ByteReader::bytes(std::size_t n) {
    // TODO: check there are n bytes left, then return them and move past them.
    (void)n;
    return {};
}

std::uint8_t ByteReader::u8() {
    // TODO
    return 0;
}

std::uint16_t ByteReader::le16() {
    // TODO
    return 0;
}

std::uint32_t ByteReader::le32() {
    // TODO
    return 0;
}

std::uint16_t ByteReader::be16() {
    // TODO
    return 0;
}

std::uint32_t ByteReader::be32() {
    // TODO
    return 0;
}

void ByteReader::skip(std::size_t n) {
    // TODO
    (void)n;
}

void ByteReader::seek(std::size_t pos) {
    // TODO
    (void)pos;
}
