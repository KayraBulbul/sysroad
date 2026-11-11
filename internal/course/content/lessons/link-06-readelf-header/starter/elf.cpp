#include "elf.hpp"

#include <format>
#include <stdexcept>

#include "bytes.hpp"

ElfHeader parse_header(std::string_view b) {
    // TODO
    (void)b;
    return {};
}

std::vector<Segment> parse_segments(std::string_view b, const ElfHeader& h) {
    // TODO
    (void)b;
    (void)h;
    return {};
}

std::string type_name(std::uint16_t type) {
    // TODO
    (void)type;
    return "";
}

std::string machine_name(std::uint16_t machine) {
    // TODO
    (void)machine;
    return "";
}

std::string segment_type(std::uint32_t type) {
    // TODO
    (void)type;
    return "";
}

std::string segment_flags(std::uint32_t flags) {
    // TODO
    (void)flags;
    return "";
}
