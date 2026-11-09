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

TEST_CASE("the last word counts without a separator after it") {
    HashMap<String, std::size_t> counts;
    std::istringstream in("end");
    count_words(in, counts);
    CHECK(*counts.find(String("end")) == 1);
}

TEST_CASE("counts add up across calls") {
    HashMap<String, std::size_t> counts;
    std::istringstream a("one two");
    std::istringstream b("two three\n");
    count_words(a, counts);
    count_words(b, counts);
    CHECK(show(top_words(counts, 5)) == "two=2 one=1 three=1 ");
}

TEST_CASE("digits, bytes above 127 and empty input") {
    HashMap<String, std::size_t> counts;
    std::istringstream in(std::string("abc123def na\xc3\xafve \0x", 20));
    count_words(in, counts);
    CHECK(show(top_words(counts, 10)) == "abc=1 def=1 na=1 ve=1 x=1 ");
    HashMap<String, std::size_t> none;
    std::istringstream empty("  ,,, 123 ");
    count_words(empty, none);
    CHECK(none.size() == 0);
    CHECK(top_words(none, 5).size() == 0);
}

TEST_CASE("n of zero, and long words") {
    HashMap<String, std::size_t> counts;
    std::string longword(100, 'z');
    std::istringstream in(longword + " " + longword + " short");
    count_words(in, counts);
    CHECK(top_words(counts, 0).size() == 0);
    Vector<WordCount> top = top_words(counts, 1);
    REQUIRE(top.size() == 1);
    CHECK(top[0].word == longword);
    CHECK(top[0].count == 2);
}

TEST_CASE("many distinct words") {
    HashMap<String, std::size_t> counts;
    std::string text;
    for (int i = 0; i < 2000; ++i) {
        std::string w;
        for (int k = i; k > 0 || w.empty(); k /= 26) {
            w += char('a' + k % 26);
        }
        text += w + " ";
        if (i % 7 == 0) {
            text += w + " ";
        }
    }
    std::istringstream in(text);
    count_words(in, counts);
    CHECK(counts.size() == 2000);
    Vector<WordCount> top = top_words(counts, 3);
    CHECK(top[0].count == 2);
    CHECK(top[2].count == 2);
    CHECK(top[0].word < top[1].word);
}
