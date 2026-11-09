#include "doctest.h"

#include <sstream>
#include <string>

#include "wordfreq.hpp"

static std::string show(const Vector<WordCount>& v) {
    std::string out;
    for (const WordCount& wc : v) {
        out += std::string(std::string_view(wc.word)) + "=" + std::to_string(wc.count) + " ";
    }
    return out;
}

TEST_CASE("count_words lowercases and splits on non-letters") {
    HashMap<String, std::size_t> counts;
    std::istringstream in("The cat, the CAT-flap and the cat's toy");
    count_words(in, counts);
    REQUIRE(counts.find(String("the")) != nullptr);
    CHECK(*counts.find(String("the")) == 3);
    CHECK(*counts.find(String("cat")) == 3);
    CHECK(*counts.find(String("s")) == 1);
    CHECK(counts.find(String("The")) == nullptr);
}

TEST_CASE("top_words orders by count, then by word") {
    HashMap<String, std::size_t> counts;
    std::istringstream in("b a c b a d b");
    count_words(in, counts);
    CHECK(show(top_words(counts, 10)) == "b=3 a=2 c=1 d=1 ");
    CHECK(show(top_words(counts, 2)) == "b=3 a=2 ");
}
