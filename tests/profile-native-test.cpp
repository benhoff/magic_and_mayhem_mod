#include "profile.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
std::vector<std::uint8_t> bytes(const std::string& text) { return {text.begin(), text.end()}; }
class MemoryFile final : public AssetFile {
public:
    std::vector<std::uint8_t> data;
    std::size_t cursor = 0;
    bool failSeek = false, failRead = false;
    explicit MemoryFile(std::string text) : AssetFile("generated.ini", {}), data(bytes(text)) {}
    Result<std::int64_t> size() override { return std::int64_t(data.size()); }
    Result<std::int64_t> position() override { return std::int64_t(cursor); }
    Status seek(std::int64_t n) override {
        if (failSeek || n < 0 || std::uint64_t(n) > data.size())
            return failure(ErrorCode::ioError, "seek", "injected");
        cursor = std::size_t(n); return std::monostate{};
    }
    ReadResult read(void* destination, std::int64_t capacity) override {
        if (failRead) return {0, failure(ErrorCode::ioError, "read", "injected")};
        const auto n = std::min({std::size_t(capacity), data.size() - cursor, std::size_t(3)});
        std::copy_n(data.data() + cursor, n, static_cast<std::uint8_t*>(destination));
        cursor += n; return {std::int64_t(n), std::nullopt};
    }
};
}
int main() try {
    const auto profile = ProfileSnapshot::parse(bytes(
        "; generated ASCII policy\r\n[Sounds] ; header comment\r\n"
        "Tone = alpha\r\nQuoted = \" other \"\r\nEmpty=\r\nLiteral=x ; inline\r\n"
        "[Map]\n10\n20\n[Numbers]\nDecimal=42\nZero=0\nMax=4294967295\nQuoted='42'\n"
        "Hex=0x10\nNegative=-1\nSuffix=3junk\nEmpty=\nOverflow=4294967296\n"));
    check(profile.value("sOuNdS", "tOnE", 128) == "alpha", "ASCII case folding");
    check(profile.value("Sounds", "Quoted", 128) == " other ", "quote stripping retains interior spaces");
    check(profile.value("Sounds", "Empty", 128, "fallback").empty(), "empty differs from missing");
    check(profile.value("Sounds", "Literal", 128) == "x ; inline", "inline comment retained as data");
    check(profile.value("Map", "10", 128, "bare") == "bare", "bare key has no string value");
    check(profile.value("missing", "key", 128, "  fallback  ") == "  fallback", "default trailing spaces");
    check(profile.value("missing", "key", 128, "fallback\t") == "fallback\t", "default trailing tab");
    check(profile.value("Sounds", "Tone", 4) == "alp" && profile.value("Sounds", "Tone", 1).empty(), "string truncation");
    bool refused = false;
    try { profile.value("Sounds", "Tone", 0); } catch (const std::invalid_argument&) { refused = true; }
    check(refused, "zero string capacity");
    const auto list = profile.section("Map", 8);
    check(list.returned == 6 && list.entries == std::vector<std::string>({"10", "20"}), "ordered bare section");
    for (unsigned capacity : {2u, 4u, 7u}) {
        const auto truncated = profile.section("Map", capacity);
        check(truncated.returned == capacity - 2 && truncated.entries.empty(), "short/exact section marked incomplete");
    }
    check(profile.section("absent", 8).returned == 0 && profile.section("absent", 8).entries.empty(), "missing section");
    refused = false;
    try { profile.section("Map", 1); } catch (const std::invalid_argument&) { refused = true; }
    check(refused, "section requires two terminators");
    check(profile.integer("Numbers", "Decimal", 17) == 42 && profile.integer("Numbers", "Zero", 17) == 0
          && profile.integer("Numbers", "Max", 17) == 0xffffffffu
          && profile.integer("Numbers", "Quoted", 17) == 42
          && profile.integer("Numbers", "absent", 17) == 17, "complete unsigned decimal and missing fallback");
    for (const auto* key : {"Hex", "Negative", "Suffix", "Empty", "Overflow"}) {
        refused = false;
        try { profile.integer("Numbers", key, 17); } catch (const std::invalid_argument&) { refused = true; }
        check(refused, "invalid unsigned decimal");
    }
    for (const auto& text : {std::string("[x]\na=1\nA=2\n"), std::string("[x]\n[X]\n"),
             std::string("[broken\n"), std::string("[x] suffix\n"), std::string("[]\n"),
             std::string("key=value\n"), std::string("[x]\n=empty-key\n"),
             std::string("[x]\na=\0bad", 11), std::string("[x]\na=\xff"), std::string("[x]\na=\x7f"),
             std::string(std::size_t(ProfileInputLimit) + 1, ' ')}) {
        refused = false;
        try { ProfileSnapshot::parse(bytes(text)); } catch (const std::invalid_argument&) { refused = true; }
        check(refused, "malformed/duplicate/non-ASCII/oversized input");
    }
    ProfileSnapshot owned;
    { MemoryFile file("[Owned]\nkey=value\n"); file.cursor = 7;
      auto result = loadProfile(file); check(std::holds_alternative<ProfileSnapshot>(result), "short-read load from start");
      owned = std::get<ProfileSnapshot>(std::move(result)); }
    check(owned.value("Owned", "key", 20) == "value", "snapshot outlives source");
    for (bool seekFailure : {false, true}) {
        MemoryFile file("[x]\na=1\n"); file.failSeek = seekFailure; file.failRead = !seekFailure;
        auto result = loadProfile(file);
        check(std::holds_alternative<Error>(result) && std::get<Error>(result).code == ErrorCode::ioError, "I/O error propagation");
    }
    MemoryFile malformed("[bad\n");
    check(std::holds_alternative<Error>(loadProfile(malformed)), "parse error returned through file API");
    std::cout << "Native profile queries, capacity boundaries, malformed input, ownership and I/O failures passed\n";
    return 0;
} catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
