// The course's version of this file. You won't need to change it.
#include <cctype>
#include <string>

#include "shell.hpp"

static bool is_redirect(const Token& t) {
    return t.kind == Token::Op && (t.text[0] == '<' || t.text[0] == '>');
}

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

static bool empty(const Command& c) {
    return c.argv.empty() && c.assignments.empty() && c.redirects.empty();
}

List parse(const std::vector<Token>& tokens) {
    List list;
    Step step;
    Command cmd;
    auto end_command = [&](const std::string& op) {
        if (empty(cmd)) {
            throw SyntaxError("unexpected " + op);
        }
        step.pipeline.commands.push_back(std::move(cmd));
        cmd = Command{};
    };
    size_t i = 0;
    while (i < tokens.size()) {
        const Token& t = tokens[i];
        if (t.kind == Token::Word) {
            if (cmd.argv.empty() && t.assignment) {
                size_t eq = t.text.find('=');
                cmd.assignments.emplace_back(t.text.substr(0, eq), t.text.substr(eq + 1));
            } else {
                cmd.argv.push_back(t.text);
            }
            if (!step.pipeline.text.empty()) {
                step.pipeline.text += ' ';
            }
            step.pipeline.text += t.text;
            ++i;
            continue;
        }
        if (is_redirect(t)) {
            std::string fd = t.fd != -1 ? std::to_string(t.fd) : "";
            cmd.redirects.push_back(read_redirect(tokens, i));
            step.pipeline.text += " " + fd + t.text + tokens[i - 1].text;
            continue;
        }
        end_command(t.text);
        ++i;
        if (t.text == "|") {
            step.pipeline.text += " |";
            continue;
        }
        step.pipeline.background = t.text == "&";
        step.next = t.text == "&&" ? Step::And : t.text == "||" ? Step::Or : Step::Then;
        list.push_back(std::move(step));
        step = Step{};
    }
    if (!empty(cmd)) {
        end_command("");
        list.push_back(std::move(step));
    } else if (!step.pipeline.commands.empty()) {
        throw SyntaxError("unexpected end of line after |");
    } else if (!list.empty() && (list.back().next == Step::And || list.back().next == Step::Or)) {
        throw SyntaxError("unexpected end of line");
    }
    if (!list.empty() && list.back().next == Step::Then) {
        list.back().next = Step::End;
    }
    return list;
}
