#include "minimake.hpp"

#include <stdexcept>

std::vector<std::string> plan(const Rules& rules, const Mtimes& mtimes, const std::string& goal) {
    // TODO: visit goal's dependencies first, then decide whether goal needs
    // its command. A helper that visits one target is the easy way in.
    (void)rules;
    (void)mtimes;
    (void)goal;
    throw std::logic_error("plan isn't written yet");
}
