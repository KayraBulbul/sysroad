#include "doctest.h"

#include <algorithm>
#include <map>

#include "reference.hpp"
#include "wordfreq.hpp"

static std::vector<WordCount> sorted(std::vector<WordCount> v) {
    std::sort(v.begin(), v.end(),
              [](const WordCount& a, const WordCount& b) { return a.word < b.word; });
    return v;
}

TEST_CASE("split_words edge cases") {
    CHECK(split_words("").empty());
    CHECK(split_words("  ...  123 \n").empty());
    CHECK(split_words("don't") == std::vector<std::string>{"don", "t"});
    CHECK(split_words("x1y") == std::vector<std::string>{"x", "y"});
    CHECK(split_words("MiXeD") == std::vector<std::string>{"mixed"});
    CHECK(split_words("caf\xc3\xa9 ok") == std::vector<std::string>{"caf", "ok"});
}

TEST_CASE("count_words matches a simple count") {
    std::vector<std::string> words = split_words(generate_text(20'000, 3'000));
    std::map<std::string, long> want;
    for (const std::string& w : words) {
        ++want[w];
    }
    std::vector<WordCount> got = sorted(count_words(words));
    REQUIRE(got.size() == want.size());
    std::size_t i = 0;
    for (const auto& [word, count] : want) {
        CHECK(got[i].word == word);
        CHECK(got[i].count == count);
        ++i;
    }
}

TEST_CASE("count_words of nothing") {
    CHECK(count_words({}).empty());
}

TEST_CASE("top_n breaks ties alphabetically and copes with small n") {
    std::vector<WordCount> in{{"b", 3}, {"a", 3}, {"c", 5}, {"d", 1}};
    CHECK(top_n(in, 0).empty());
    CHECK(top_n(in, 10).size() == 4);
    std::vector<WordCount> want{{"c", 5}, {"a", 3}, {"b", 3}};
    CHECK(top_n(in, 3) == want);
}

TEST_CASE("count_words keeps up with a std::map on 60,000 words") {
    std::vector<std::string> words = split_words(generate_text(60'000, 15'000));
    double map_s = seconds([&] { reference_count(words); });
    double yours_s = seconds([&] { count_words(words); });
    CAPTURE(map_s);
    CHECK(yours_s < 3 * map_s);
}
