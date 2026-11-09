// Tries the cache and the family tree. You won't need to change this file.
#include <cstdio>
#include <memory>
#include <string>

#include "cache.hpp"
#include "family.hpp"

int main() {
    Cache cache([](const std::string& name) { return "contents of " + name; });
    auto first = cache.get("a.txt");
    std::printf("first get:   %d load%s, use_count %ld\n", cache.loads(),
                cache.loads() == 1 ? "" : "s", first.use_count());
    auto second = cache.get("a.txt");
    std::printf("second get:  %d load%s, same object: %s, use_count %ld\n", cache.loads(),
                cache.loads() == 1 ? "" : "s", first == second ? "yes" : "no", second.use_count());
    first.reset();
    second.reset();
    std::printf("let go:      %zu in use\n", cache.in_use());
    auto third = cache.get("a.txt");
    std::printf("third get:   %d loads\n\n", cache.loads());

    auto byron = std::make_shared<Person>(Person{"Byron", {}, {}});
    auto ada = std::make_shared<Person>(Person{"Ada", {}, {}});
    auto charles = std::make_shared<Person>(Person{"Charles", {}, {}});
    adopt(byron, ada);
    adopt(ada, charles);
    std::printf("%s\n", lineage(*charles).c_str());

    std::weak_ptr<Person> watch = byron;
    ada.reset();
    charles.reset();
    byron.reset();
    std::printf("after letting go of the family: %s\n",
                watch.expired() ? "everyone was freed" : "Byron is still alive: a cycle leaked");
}
