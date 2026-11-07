#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <istream>
#include <map>
#include <stdexcept>
#include <string>

// A bad line in a config file. what() is "line N: message".
class ConfigError : public std::runtime_error {
  public:
    ConfigError(int line, const std::string& msg);
    int line() const {
        return line_;
    }

  private:
    int line_;
};

// Reads "key = value" lines. Throws ConfigError on the first bad line.
std::map<std::string, std::string> parse_config(std::istream& in);
