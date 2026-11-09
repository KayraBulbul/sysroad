#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <span>

// Reads a byte buffer from front to back. A read that needs more bytes than
// are left returns 0 (or an empty span), doesn't move, and clears ok() for
// good, so you can do a run of reads and check ok() once.
class ByteReader {
  public:
    explicit ByteReader(std::span<const unsigned char> data) : data_(data) {}

    std::size_t pos() const {
        return pos_;
    }
    std::size_t remaining() const {
        return data_.size() - pos_;
    }
    bool ok() const {
        return ok_;
    }

    // The next n bytes, as a view into the buffer.
    std::span<const unsigned char> bytes(std::size_t n);

    std::uint8_t u8();
    std::uint16_t le16();
    std::uint32_t le32();
    std::uint16_t be16();
    std::uint32_t be32();

    // Moves forward n bytes.
    void skip(std::size_t n);
    // Moves to an absolute position, from 0 to the buffer's size.
    void seek(std::size_t pos);

  private:
    std::span<const unsigned char> data_;
    std::size_t pos_ = 0;
    bool ok_ = true;
};
