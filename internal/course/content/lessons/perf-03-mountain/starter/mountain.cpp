#include "mountain.hpp"

#include "bench.hpp"

long read_stride(const long* data, std::size_t elems, std::size_t stride) {
    // TODO
    (void)data;
    (void)elems;
    (void)stride;
    return 0;
}

std::size_t bytes_read(std::size_t elems, std::size_t stride) {
    // TODO
    (void)elems;
    (void)stride;
    return 0;
}

std::string human_size(std::size_t bytes) {
    // TODO
    (void)bytes;
    return "";
}

Mountain build_mountain(const std::vector<long>& data, const std::vector<std::size_t>& sizes,
                        const std::vector<std::size_t>& strides, const Timer& time) {
    // TODO
    (void)data;
    (void)time;
    return {sizes, strides, {}};
}
