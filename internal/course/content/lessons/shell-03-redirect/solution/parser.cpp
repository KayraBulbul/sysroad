#include <cctype>

#include "conch.hpp"

static bool all_digits(const std::string& s) {
    if (s.empty() || s.size() > 4) {
        return false;
    }
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

// Reads the redirection at tokens[i] and its target, and moves i past both.
static Redirect read_redirect(const std::vector<Token>& tokens, size_t& i) {
    const Token& op = tokens[i];
    if (i + 1 == tokens.size() || tokens[i + 1].kind != Token::Word) {
        throw SyntaxError("expected a file name after " + op.text);
    }
    const std::string& target = tokens[i + 1].text;
    i += 2;
    Redirect r;
    r.fd = op.fd != -1 ? op.fd : (op.text[0] == '<' ? 0 : 1);
    if (op.text == "<&" || op.text == ">&") {
        if (!all_digits(target)) {
            throw SyntaxError("expected an fd number after " + op.text);
        }
        r.op = Redirect::Dup;
        r.from = std::stoi(target);
        return r;
    }
    r.op = op.text == "<" ? Redirect::Read : op.text == ">>" ? Redirect::Append : Redirect::Write;
    r.path = target;
    return r;
}

Command parse_command(const std::vector<Token>& tokens) {
    Command c;
    size_t i = 0;
    while (i < tokens.size()) {
        if (tokens[i].kind == Token::Word) {
            c.argv.push_back(tokens[i].text);
            ++i;
        } else {
            c.redirects.push_back(read_redirect(tokens, i));
        }
    }
    return c;
}
