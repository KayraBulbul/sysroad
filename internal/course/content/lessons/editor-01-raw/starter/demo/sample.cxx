// A sample file to try kite on: it's indented with tabs, it has a line
// too long for most screens, and more lines than fit on one. It's .cxx,
// not .cpp, because every .cpp file in the folder is compiled into kite.
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/* Counts the words in a file and prints the ten most common,
   the way `tr | sort | uniq -c | sort -rn | head` would. */

struct WordCount {
	std::string word;
	long count = 0;
};

static std::string lowercase(std::string s) {
	for (char& c : s) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}
	return s;
}

static std::map<std::string, long> count_words(std::istream& in) {
	std::map<std::string, long> counts;
	std::string word;
	char c;
	while (in.get(c)) {
		if (std::isalpha(static_cast<unsigned char>(c))) {
			word += c;
		} else if (!word.empty()) {
			++counts[lowercase(word)];
			word.clear();
		}
	}
	if (!word.empty()) {
		++counts[lowercase(word)];
	}
	return counts;
}

static std::vector<WordCount> top(const std::map<std::string, long>& counts, std::size_t n) {
	std::vector<WordCount> all;
	for (const auto& [word, count] : counts) {
		all.push_back({word, count});
	}
	std::sort(all.begin(), all.end(), [](const WordCount& a, const WordCount& b) {
		return a.count != b.count ? a.count > b.count : a.word < b.word;
	});
	if (all.size() > n) {
		all.resize(n);
	}
	return all;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "usage: top10 FILE\n";
		return 2;
	}
	std::ifstream in(argv[1]);
	if (!in) {
		std::cerr << "top10: " << argv[1] << ": can't open the file, so there's nothing to count, and this message is long enough to need scrolling\n";
		return 1;
	}
	for (const WordCount& wc : top(count_words(in), 10)) {
		std::cout << wc.count << '\t' << wc.word << '\n';
	}
	return 0;
}
