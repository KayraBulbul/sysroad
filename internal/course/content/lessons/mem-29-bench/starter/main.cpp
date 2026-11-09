// Races your containers against the standard library's. You won't need to change this file.
#include <cstdio>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "bench.hpp"
#include "hashmap.hpp"
#include "string.hpp"
#include "vector.hpp"

static void race(const char* what, double std_ns, double yours_ns) {
    std::printf("%-22s %10.1f µs %10.1f µs   ", what, std_ns / 1000, yours_ns / 1000);
    if (std_ns <= 0 || yours_ns <= 0) {
        std::printf("(no timing yet)\n");
    } else if (yours_ns <= std_ns) {
        std::printf("yours is %.1fx faster\n", std_ns / yours_ns);
    } else {
        std::printf("yours is %.1fx slower\n", yours_ns / std_ns);
    }
}

int main() {
    const int n = 100'000;
    std::printf("%-22s %13s %13s\n", "", "std", "yours");

    race("push_back 100k ints",
         ns_per_call(
             [] {
                 std::vector<int> v;
                 for (int i = 0; i < n; ++i) {
                     v.push_back(i);
                 }
                 do_not_optimize(v.data());
             },
             5, 9),
         ns_per_call(
             [] {
                 Vector<int> v;
                 for (int i = 0; i < n; ++i) {
                     v.push_back(i);
                 }
                 do_not_optimize(v.data());
             },
             5, 9));

    race("100k short strings",
         ns_per_call(
             [] {
                 for (int i = 0; i < n; ++i) {
                     std::string s("short");
                     s += "er";
                     do_not_optimize(s.data());
                 }
             },
             3, 9),
         ns_per_call(
             [] {
                 for (int i = 0; i < n; ++i) {
                     String s("short");
                     s += "er";
                     do_not_optimize(s.data());
                 }
             },
             3, 9));

    std::unordered_map<int, int> um;
    HashMap<int, int> hm;
    for (int i = 0; i < n; ++i) {
        um[i * 7] = i;
        hm[i * 7] = i;
    }
    race("100k hash lookups",
         ns_per_call(
             [&] {
                 long sum = 0;
                 for (int i = 0; i < n; ++i) {
                     sum += um.find(i * 7)->second;
                 }
                 do_not_optimize(sum);
             },
             5, 9),
         ns_per_call(
             [&] {
                 long sum = 0;
                 for (int i = 0; i < n; ++i) {
                     sum += *hm.find(i * 7);
                 }
                 do_not_optimize(sum);
             },
             5, 9));

    std::vector<int> vec(1'000'000, 1);
    std::list<int> lst(vec.begin(), vec.end());
    double vec_ns = ns_per_call(
        [&] {
            long sum = 0;
            for (int x : vec) {
                sum += x;
            }
            do_not_optimize(sum);
        },
        3, 9);
    double list_ns = ns_per_call(
        [&] {
            long sum = 0;
            for (int x : lst) {
                sum += x;
            }
            do_not_optimize(sum);
        },
        3, 9);
    std::printf("\nsumming 1M ints: std::vector %.1f µs, std::list %.1f µs (%.0fx slower)\n",
                vec_ns / 1000, list_ns / 1000, vec_ns > 0 ? list_ns / vec_ns : 0);
}
