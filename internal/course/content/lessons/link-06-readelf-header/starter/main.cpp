// readelf -h -l, for 64-bit little-endian ELF files. You won't need to
// change this file.
//
//   readelf FILE          any ELF file, like /bin/ls on Linux
//   readelf x86-64-exe    one of the built-in samples: x86-64-exe,
//                         x86-64.o or arm64.o (the default is x86-64-exe)
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>

#include "elf.hpp"
#include "samples.hpp"

static std::string load(const char* name) {
    if (std::strcmp(name, "x86-64-exe") == 0) {
        return std::string(bytes_of(sample_x86_64_exe));
    }
    if (std::strcmp(name, "x86-64.o") == 0) {
        return std::string(bytes_of(sample_x86_64_o));
    }
    if (std::strcmp(name, "arm64.o") == 0) {
        return std::string(bytes_of(sample_arm64_o));
    }
    std::ifstream in(name, std::ios::binary);
    if (!in) {
        throw std::runtime_error(std::string("can't open ") + name);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void print_header(const ElfHeader& h) {
    std::printf("ELF Header:\n");
    std::printf("  %-35s %s\n", "Class:", "ELF64");
    std::printf("  %-35s %s\n", "Data:", "2's complement, little endian");
    std::printf("  %-35s %u\n", "OS/ABI:", h.os_abi);
    std::printf("  %-35s %s\n", "Type:", type_name(h.type).c_str());
    std::printf("  %-35s %s\n", "Machine:", machine_name(h.machine).c_str());
    std::printf("  %-35s 0x%llx\n", "Entry point address:", (unsigned long long)h.entry);
    std::printf("  %-35s %llu (bytes into file)\n",
                "Start of program headers:", (unsigned long long)h.phoff);
    std::printf("  %-35s %llu (bytes into file)\n",
                "Start of section headers:", (unsigned long long)h.shoff);
    std::printf("  %-35s 0x%x\n", "Flags:", h.flags);
    std::printf("  %-35s %u (bytes)\n", "Size of this header:", h.ehsize);
    std::printf("  %-35s %u (bytes)\n", "Size of program headers:", h.phentsize);
    std::printf("  %-35s %u\n", "Number of program headers:", h.phnum);
    std::printf("  %-35s %u (bytes)\n", "Size of section headers:", h.shentsize);
    std::printf("  %-35s %u\n", "Number of section headers:", h.shnum);
    std::printf("  %-35s %u\n", "Section header string table index:", h.shstrndx);
}

static void print_segments(std::string_view b, const std::vector<Segment>& segs) {
    if (segs.empty()) {
        std::printf("\nThere are no program headers in this file.\n");
        return;
    }
    std::printf("\nProgram Headers:\n");
    std::printf("  %-14s %-8s %-18s %-8s %-8s %-3s %s\n", "Type", "Offset", "VirtAddr", "FileSiz",
                "MemSiz", "Flg", "Align");
    for (const Segment& s : segs) {
        std::printf("  %-14s 0x%06llx 0x%016llx 0x%06llx 0x%06llx %s 0x%llx\n",
                    segment_type(s.type).c_str(), (unsigned long long)s.offset,
                    (unsigned long long)s.vaddr, (unsigned long long)s.filesz,
                    (unsigned long long)s.memsz, segment_flags(s.flags).c_str(),
                    (unsigned long long)s.align);
        if (s.type == 3 && s.offset < b.size()) {
            // INTERP: the path of the dynamic loader, ending in '\0'.
            std::string path(b.substr(s.offset, s.filesz));
            std::printf("      [Requesting program interpreter: %s]\n", path.c_str());
        }
    }
}

int main(int argc, char** argv) {
    try {
        std::string b = load(argc > 1 ? argv[1] : "x86-64-exe");
        ElfHeader h = parse_header(b);
        print_header(h);
        print_segments(b, parse_segments(b, h));
    } catch (const std::exception& e) {
        std::fprintf(stderr, "readelf: %s\n", e.what());
        return 1;
    }
}
