#pragma once
// Two classes whose moves aren't what they look like.

#include <string>
#include <utility>
#include <vector>

// A student's name and marks.
struct Record {
    std::string name;
    std::vector<int> scores;

    Record(std::string n, std::vector<int> s) : name(std::move(n)), scores(std::move(s)) {}
    Record(const Record&) = default;
    Record& operator=(const Record&) = default;
    Record(Record&& o) : name(std::move(o.name)), scores(std::move(o.scores)) {}
    Record& operator=(Record&& o) {
        name = std::move(o.name);
        scores = std::move(o.scores);
        return *this;
    }
};

// One line of a log.
struct Line {
    static inline int copies = 0;  // for debugging
    std::string text;

    explicit Line(std::string t) : text(std::move(t)) {}
    Line(const Line& o) : text(o.text) {
        ++copies;
    }
    Line& operator=(const Line& o) = default;
};
