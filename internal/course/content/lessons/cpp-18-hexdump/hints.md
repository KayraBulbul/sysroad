`std::format("{:08x}", offset)` is 8 zero-padded hex digits.
`std::format("{:02x} ", static_cast<unsigned char>(b))` prints one byte.
Without the cast, a byte like 0xff is a negative `char`.
---
Loop `i` from 0 to 15. When `i % 8 == 0`, add one extra space (that's the
second space after the offset, and the gap in the middle). Add the byte if
`i < bytes.size()`, or else three spaces.
---
`feed`: `pending_ += bytes;`, then while at least 16 bytes are waiting,
print `hex_line(offset_, first 16)`, add 16 to `offset_`, and drop them.
Erasing from the front of a string each time is slow for big chunks.
Count how many you printed and do one `pending_.erase(0, done)` at the
end.
