#include "count.hpp"

#include <unistd.h>

// Correct and slow: one byte at a time, 4 KB per read. Make it fast.

void count_chunk(CountState& st, const char* p, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        unsigned char c = static_cast<unsigned char>(p[i]);
        if (c == '\n') {
            ++st.counts.lines;
        }
        bool space = is_space(c);
        if (!space && !st.in_word) {
            ++st.counts.words;
        }
        st.in_word = !space;
    }
    st.counts.bytes += n;
}

bool count_fd(int fd, Counts& out) {
    CountState st;
    char buf[4096];
    for (;;) {
        ssize_t got = read(fd, buf, sizeof buf);
        if (got < 0) {
            out = st.counts;
            return false;
        }
        if (got == 0) {
            break;
        }
        count_chunk(st, buf, static_cast<std::size_t>(got));
    }
    out = st.counts;
    return true;
}
