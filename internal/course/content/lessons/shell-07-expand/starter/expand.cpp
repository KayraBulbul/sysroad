#include <unistd.h>

#include <cctype>
#include <string>

#include "shell.hpp"

std::optional<std::string> expand_word(const std::string& word, const Shell& sh) {
    // TODO: quotes, backslashes, $NAME, ${NAME}, $?, $$, $!, and ~.
    (void)sh;
    return word;
}
