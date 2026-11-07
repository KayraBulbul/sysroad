// A small demo of your templates. Run prints what it shows.
#include <iostream>
#include <string>

#include "generic.hpp"

int main() {
    std::vector<int> nums{3, 9, 4};
    std::vector<std::string> words{"pear", "apple", "zucchini"};
    std::cout << "max of nums: " << max_of(nums).value_or(-1) << '\n';
    std::cout << "max of words: " << max_of(words).value_or("(none)") << '\n';

    Stack<std::string> s;
    s.push("first");
    s.push("second");
    std::cout << "stack size: " << s.size() << '\n';
    while (auto top = s.pop()) {
        std::cout << "popped " << *top << '\n';
    }

    auto odd = count_where(nums.begin(), nums.end(), [](int n) { return n % 2 != 0; });
    std::cout << "odd numbers: " << odd << '\n';
    return 0;
}
