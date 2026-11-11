#include "doctest.h"

#include <algorithm>

#include "reference.hpp"
#include "wordfreq.hpp"

static std::vector<WordCount> sorted(std::vector<WordCount> v) {
    std::sort(v.begin(), v.end(), [](const WordCount& a, const WordCount& b) { return a.word < b.word; });
    return v;
}

TEST_CASE("split_words") {
    CHECK(split_words("The cat, the HAT.") == std::vector<std::string>{"the", "cat", "the", "hat"});
}

TEST_CASE("count_words") {
    std::vector<WordCount> want{{"cat", 1}, {"hat", 1}, {"the", 2}};
    CHECK(sorted(count_words({"the", "cat", "the", "hat"})) == want);
}

TEST_CASE("top_n") {
    std::vector<WordCount> want{{"the", 2}, {"cat", 1}};
    CHECK(top_n({{"the", 2}, {"hat", 1}, {"cat", 1}}, 2) == want);
}

TEST_CASE("count_words on 40,000 words is about as fast as a std::map") {
    std::vector<std::string> words = split_words(generate_text(40'000, 10'000));
    double map_s = seconds([&] { reference_count(words); });
    double yours_s = seconds([&] { CHECK(count_words(words).size() > 5'000); });
    CAPTURE(map_s);
    CHECK(yours_s < 3 * map_s);
}
