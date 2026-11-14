#pragma once
#include <string>
#include <string_view>

int count_words(std::string_view text);
int count_lines(const std::string& text);
std::string report(std::string_view text);
