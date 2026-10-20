#include "../reconstruction/rendering/distortion_word_read.hpp"
#include <iostream>
#include <limits>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

using mnm::reconstruction::distortionWordRead;
static unsigned checks;
static void check(bool ok) { ++checks; if (!ok) throw std::runtime_error("word-read control failed"); }
int main(int argc, char** argv) try {
    if (argc == 4) {
        const auto logical = std::stoul(argv[1]), tail = std::stoul(argv[2]), distance = std::stoul(argv[3]);
        if (!logical || logical > 2048 * 2048 * 2 || tail > 2 || distance > 2)
            throw std::invalid_argument("fixture bounds");
        std::vector<std::uint8_t> bytes(logical + tail, 0xa5);
        for (std::size_t i = 0; i < logical / 2; ++i) {
            const auto word = static_cast<std::uint16_t>(i * 97 + 13);
            bytes[2 * i] = word; bytes[2 * i + 1] = word >> 8;
        }
        const auto r = distortionWordRead(bytes.data(), bytes.size(), logical, logical / 2 - 1 - distance);
        std::cout << "{\"readable\":" << (r.operand ? "true" : "false")
                  << ",\"source_byte_offset\":" << r.byteOffset
                  << ",\"crosses_logical_end\":" << (r.crossesLogicalEnd ? "true" : "false");
        if (r.operand) std::cout << ",\"operand\":" << *r.operand << ",\"stored_word\":" << *r.storedWord();
        if (r.firstUnreadable) std::cout << ",\"first_unreadable\":" << *r.firstUnreadable;
        std::cout << "}\n";
        return 0;
    }
    if (argc != 1) throw std::invalid_argument("expected no args or logical-bytes tail distance");
    std::uint8_t bytes[]{0x34, 0x12, 0x78, 0x56, 0xab, 0xcd};
    const auto interior = distortionWordRead(bytes, 6, 4, 0);
    check(interior.operand == 0x56781234u && interior.storedWord() == 0x1234 && !interior.crossesLogicalEnd);
    const auto final = distortionWordRead(bytes, 6, 4, 1);
    check(final.operand == 0xcdab5678u && final.storedWord() == 0x5678 && final.crossesLogicalEnd);
    for (unsigned tail = 0; tail < 2; ++tail) {
        const auto fault = distortionWordRead(bytes, 4 + tail, 4, 1);
        check(!fault.operand && !fault.storedWord() && fault.firstUnreadable == 4 + tail);
    }
    auto refuse = [&](const std::uint8_t* p, std::size_t readable, std::size_t logical, std::size_t index) {
        try { distortionWordRead(p, readable, logical, index); }
        catch (const std::invalid_argument&) { ++checks; return; }
        throw std::runtime_error("invalid source accepted");
    };
    refuse(nullptr, 6, 4, 0); refuse(bytes, 6, 0, 0); refuse(bytes, 6, 3, 0);
    refuse(bytes, 3, 4, 0); refuse(bytes, 6, 4, 2);
    refuse(bytes, 6, 4, std::numeric_limits<std::size_t>::max());
    const auto page = static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
    auto* mapped = static_cast<std::uint8_t*>(mmap(nullptr, page * 2, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (mapped == MAP_FAILED || mprotect(mapped + page, page, PROT_NONE)) throw std::runtime_error("guard page");
    mapped[page - 2] = 0x34; mapped[page - 1] = 0x12;
    check(!distortionWordRead(mapped, page, page, page / 2 - 1).operand);
    // An odd base gives exactly one readable byte after the logical WORD.
    check(!distortionWordRead(mapped + 1, page - 1, page - 2, (page - 2) / 2 - 1).operand);
    check(distortionWordRead(mapped, page, page, page / 2 - 2).operand.has_value());
    if (munmap(mapped, page * 2)) throw std::runtime_error("unmap");
    std::cout << "{\"success\":true,\"native_controls\":" << checks << "}\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
