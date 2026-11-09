#pragma once
// You won't need to change this file.

#include <string>
#include <string_view>

// "Hello, NAME!", or "Hello, NAME?" when name already ends in a question.
// A name starting with "--" isn't a name, so it gets "Hello, stranger!".
std::string greet(std::string_view name);
