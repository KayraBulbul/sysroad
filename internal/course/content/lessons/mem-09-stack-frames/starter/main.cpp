// Watch the stack grow, then walk some deep structures. You won't need to change this file.
#include <cstdint>
#include <cstdio>
#include <vector>

#include "walk.hpp"

static std::uintptr_t previous = 0;

// Each call prints where its frame is: __builtin_frame_address(0) is the
// address in the frame pointer register.
static void nest(int depth) {
    auto here = reinterpret_cast<std::uintptr_t>(__builtin_frame_address(0));
    if (previous == 0) {
        std::printf("frame %d is at %#lx\n", depth, static_cast<unsigned long>(here));
    } else {
        std::printf("frame %d is at %#lx, %ld bytes below its caller\n", depth,
                    static_cast<unsigned long>(here), static_cast<long>(previous - here));
    }
    previous = here;
    if (depth < 3) {
        nest(depth + 1);
    }
}

int main() {
    nest(0);
    std::fflush(stdout);  // so this shows even if what follows crashes

    const long n = 1'000'000;
    std::vector<Node> list(n);
    for (long i = 0; i < n; ++i) {
        list[i] = {i + 1, i + 1 < n ? &list[i + 1] : nullptr};
    }
    std::printf("\nsum of a %ld-node list: %ld\n", n, sum_list(list.data()));
    std::fflush(stdout);

    // A tree that's one long chain of left children.
    std::vector<Tree> chain(n);
    for (long i = 0; i < n; ++i) {
        chain[i] = {i + 1 < n ? &chain[i + 1] : nullptr, nullptr};
    }
    std::printf("depth of a %ld-deep tree: %d\n", n, tree_depth(chain.data()));
    std::printf("nodes in it: %ld\n", count_nodes(chain.data()));
}
