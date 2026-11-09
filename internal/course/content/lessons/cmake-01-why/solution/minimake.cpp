#include "minimake.hpp"

#include <limits>
#include <stdexcept>

namespace {

// A target rebuilt in this plan counts as changed "now", newer than
// anything on disk.
constexpr long kNow = std::numeric_limits<long>::max();

struct Planner {
    const Rules& rules;
    const Mtimes& mtimes;
    std::vector<std::string> commands;
    std::map<std::string, long> done;  // target -> its time once up to date
    std::vector<std::string> path;     // the targets being visited, for cycles

    long visit(const std::string& name) {
        if (auto it = done.find(name); it != done.end()) {
            return it->second;
        }
        for (std::size_t i = 0; i < path.size(); ++i) {
            if (path[i] == name) {
                std::string msg = "cycle: ";
                for (std::size_t j = i; j < path.size(); ++j) {
                    msg += path[j] + " -> ";
                }
                throw std::runtime_error(msg + name);
            }
        }
        auto file = mtimes.find(name);
        auto rule = rules.find(name);
        if (rule == rules.end()) {
            if (file == mtimes.end()) {
                throw std::runtime_error("no rule to make " + name);
            }
            return done[name] = file->second;  // a source file
        }

        path.push_back(name);
        long newest = std::numeric_limits<long>::min();
        for (const auto& dep : rule->second.deps) {
            newest = std::max(newest, visit(dep));
        }
        path.pop_back();

        bool stale = file == mtimes.end() || newest > file->second;
        if (stale) {
            commands.push_back(rule->second.command);
            return done[name] = kNow;
        }
        return done[name] = file->second;
    }
};

}  // namespace

std::vector<std::string> plan(const Rules& rules, const Mtimes& mtimes, const std::string& goal) {
    Planner p{rules, mtimes, {}, {}, {}};
    p.visit(goal);
    return p.commands;
}
