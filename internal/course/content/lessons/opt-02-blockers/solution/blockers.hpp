#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <cstdint>
#include <vector>

// Turns the ASCII letters A-Z in the C string s into a-z.
void lowercase(char* s);

// A greyscale image: size bytes at pixels, one per pixel.
struct Image {
    std::uint8_t* pixels;
    std::size_t size;
};

// Makes light pixels dark and dark ones light: p becomes 255 - p.
void invert(Image& img);

// Pixels below level become 0, the rest 255.
void threshold(std::vector<std::uint8_t>& pixels, std::uint8_t level);
