#pragma once
// Shared by your code, main.cpp and the tests. You won't need to change this file.

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

// Adds up data[0], data[stride], data[2 * stride], … for every index
// below elems, and returns the sum.
long read_stride(const long* data, std::size_t elems, std::size_t stride);

// How many bytes read_stride(data, elems, stride) reads.
std::size_t bytes_read(std::size_t elems, std::size_t stride);

// A size for people: 16384 is "16K", 1048576 is "1M", 1073741824 is
// "1G". Sizes that aren't a whole number of K stay as bytes: "100".
std::string human_size(std::size_t bytes);

// Times one call of f, in nanoseconds. main.cpp uses your measure() from
// lesson 5.1.1. The tests use fakes.
using Timer = std::function<double(const std::function<void()>&)>;

// Read throughput, in GB/s (10^9 bytes per second), for each working-set
// size and stride: gbps[i][j] is sizes[i] (in bytes) read at strides[j]
// (in longs).
struct Mountain {
    std::vector<std::size_t> sizes;
    std::vector<std::size_t> strides;
    std::vector<std::vector<double>> gbps;
};

// Times read_stride over the first sizes[i] bytes of data at each stride,
// and fills in the GB/s. data holds at least the largest size.
Mountain build_mountain(const std::vector<long>& data, const std::vector<std::size_t>& sizes,
                        const std::vector<std::size_t>& strides, const Timer& time);
