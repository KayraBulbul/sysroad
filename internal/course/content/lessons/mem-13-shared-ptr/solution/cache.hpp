#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>

// Loads things by name and shares them. While anyone still holds what
// get(name) returned, get(name) returns that same object without loading it
// again. Once everyone has let go it's freed, and the next get loads it again.
class Cache {
  public:
    using Loader = std::function<std::string(const std::string&)>;
    explicit Cache(Loader load) : load_(std::move(load)) {}

    std::shared_ptr<const std::string> get(const std::string& name);

    // How many times the loader has run.
    int loads() const {
        return loads_;
    }

    // Forgets entries nobody holds any more, and counts the rest.
    std::size_t in_use();

  private:
    Loader load_;
    int loads_ = 0;
    std::map<std::string, std::weak_ptr<const std::string>> entries_;
};
