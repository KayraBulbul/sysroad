#include "macho.hpp"

#include <format>
#include <stdexcept>

#include "bytes.hpp"

MachHeader parse_mach_header(std::string_view b) {
    // TODO
    (void)b;
    return {};
}

std::vector<LoadCommand> load_commands(std::string_view b, const MachHeader& h) {
    // TODO
    (void)b;
    (void)h;
    return {};
}

std::vector<MachSegment> segments(std::string_view b, const std::vector<LoadCommand>& cmds) {
    // TODO
    (void)b;
    (void)cmds;
    return {};
}

std::vector<std::string> dylibs(std::string_view b, const std::vector<LoadCommand>& cmds) {
    // TODO
    (void)b;
    (void)cmds;
    return {};
}

std::vector<MachSymbol> symbols(std::string_view b, const std::vector<LoadCommand>& cmds) {
    // TODO
    (void)b;
    (void)cmds;
    return {};
}

std::string command_name(std::uint32_t cmd) {
    // TODO
    (void)cmd;
    return "";
}
