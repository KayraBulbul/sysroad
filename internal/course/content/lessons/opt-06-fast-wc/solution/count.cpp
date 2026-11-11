#include "count.hpp"

#include <cerrno>
#include <unistd.h>

#include <vector>

// 0xff in every lane holding whitespace, 0 elsewhere.
static u8x16 spaces(u8x16 v) {
    auto space = v == splat16(' ');
    auto control = (v >= splat16('\t')) & (v <= splat16('\r'));
    return reinterpret_cast<u8x16>(space | control);
}

void count_chunk(CountState& st, const char* p, std::size_t n) {
    st.counts.bytes += n;
    if (n == 0) {
        return;
    }
    // The first byte on its own: whether it starts a word depends on the
    // last chunk. After that, the byte before is always in p.
    unsigned char first = static_cast<unsigned char>(p[0]);
    st.counts.lines += first == '\n';
    st.counts.words += !is_space(first) && !st.in_word;

    std::size_t i = 1;
    const u8x16 newline = splat16('\n');
    while (i + 16 <= n) {
        u8x16 lines{}, starts{};
        for (int block = 0; block < 255 && i + 16 <= n; ++block, i += 16) {
            u8x16 v = load16(p + i);
            u8x16 before = load16(p + i - 1);  // the same bytes, one earlier
            lines -= reinterpret_cast<u8x16>(v == newline);
            starts -= ~spaces(v) & spaces(before);  // a word starts after a space
        }
        for (int lane = 0; lane < 16; ++lane) {
            st.counts.lines += lines[lane];
            st.counts.words += starts[lane];
        }
    }
    for (; i < n; ++i) {
        unsigned char c = static_cast<unsigned char>(p[i]);
        st.counts.lines += c == '\n';
        st.counts.words += !is_space(c) && is_space(static_cast<unsigned char>(p[i - 1]));
    }
    st.in_word = !is_space(static_cast<unsigned char>(p[n - 1]));
}

bool count_fd(int fd, Counts& out) {
    CountState st;
    // Big reads: one system call per 256 KB instead of per 4 KB.
    std::vector<char> buf(256 * 1024);
    for (;;) {
        ssize_t got = read(fd, buf.data(), buf.size());
        if (got < 0) {
            if (errno == EINTR) {
                continue;
            }
            out = st.counts;
            return false;
        }
        if (got == 0) {
            break;
        }
        count_chunk(st, buf.data(), static_cast<std::size_t>(got));
    }
    out = st.counts;
    return true;
}
