#include <cctype>
#include <string>

#include "shell.hpp"

Command parse_command(const std::vector<Token>& tokens) {
    Command cmd;
    for (const Token& t : tokens) {
        if (t.kind == Token::Word) {
            cmd.argv.push_back(t.text);
        }
        // TODO: an operator and the word after it are a redirection.
    }
    return cmd;
}
