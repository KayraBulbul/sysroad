// kite, for now: shows the bytes each key sends, until Ctrl-Q.
#include <cctype>
#include <cerrno>
#include <exception>
#include <iostream>
#include <system_error>
#include <unistd.h>

#include "terminal.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    try {
        RawMode raw(STDIN_FILENO);
        std::cout << "Press keys to see their bytes. Ctrl-Q quits.\r\n" << std::flush;
        while (true) {
            char c = 0;
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if (n == -1 && errno != EAGAIN && errno != EINTR) {
                throw std::system_error(errno, std::generic_category(), "read");
            }
            if (n != 1) {
                continue;  // a tenth of a second went by with no key
            }
            auto byte = static_cast<unsigned char>(c);
            std::cout << static_cast<int>(byte);
            if (std::isprint(byte)) {
                std::cout << " '" << c << "'";
            }
            std::cout << "\r\n" << std::flush;
            if (byte == ('q' & 0x1f)) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "kite: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
