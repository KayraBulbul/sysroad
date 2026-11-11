#include "sections.hpp"

#include <format>
#include <stdexcept>

#include "bytes.hpp"

std::string string_at(std::string_view b, const Section& s, std::size_t at) {
    // TODO
    (void)b;
    (void)s;
    (void)at;
    return "";
}

std::vector<Section> parse_sections(std::string_view b, const ElfHeader& h) {
    // TODO
    (void)b;
    (void)h;
    return {};
}

std::vector<Symbol> parse_symbols(std::string_view b, const std::vector<Section>& sections) {
    // TODO
    (void)b;
    (void)sections;
    return {};
}

std::string section_type(std::uint32_t type) {
    // TODO
    (void)type;
    return "";
}

std::string section_flags(std::uint64_t flags) {
    // TODO
    (void)flags;
    return "";
}

std::string symbol_type(std::uint8_t type) {
    // TODO
    (void)type;
    return "";
}

std::string symbol_bind(std::uint8_t bind) {
    // TODO
    (void)bind;
    return "";
}

std::string section_index(std::uint16_t shndx) {
    // TODO
    (void)shndx;
    return "";
}
