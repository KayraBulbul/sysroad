#pragma once
#include <string>

struct Stats {
    int words;
    int lines;
};

int count_words(const std::string& text);
int count_lines(const std::string& text);
Stats stats(const std::string& text);
