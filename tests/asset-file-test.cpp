#include "asset_file.hpp"
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace fs = std::filesystem;
using namespace mnm::assets;
static_assert(!std::is_copy_constructible_v<AssetFile>);
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class T> void rejects(const Result<T>& result, ErrorCode expected) {
    const auto* e = std::get_if<Error>(&result);
    if (!e || e->code != expected) {
        throw std::runtime_error("Expected error " + std::to_string(static_cast<int>(expected)) +
            (e ? ", got " + std::to_string(static_cast<int>(e->code)) + " in " + e->operation + ": " + e->detail : ", got success"));
    }
}
void rejects(const ReadResult& result, ErrorCode expected, std::int64_t count = 0) {
    require(result.error && result.error->code == expected && result.transferred == count, "Wrong read error/count");
}
void ok(const Status& result) { require(std::holds_alternative<std::monostate>(result), "Operation failed"); }
std::int64_t number(const Result<std::int64_t>& result) {
    require(std::holds_alternative<std::int64_t>(result), "Missing integer result");
    return std::get<std::int64_t>(result);
}
std::unique_ptr<AssetFile> opened(const AssetStore& store, const std::string& request) {
    auto result = store.open(request);
    require(std::holds_alternative<std::unique_ptr<AssetFile>>(result), "Cannot open fixture");
    return std::get<std::unique_ptr<AssetFile>>(std::move(result));
}
void write(const fs::path& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    require(bool(out), "Cannot write fixture");
}
std::vector<std::uint8_t> whole(AssetFile& file, std::int64_t limit) {
    auto result = readWhole(file, limit);
    require(std::holds_alternative<std::vector<std::uint8_t>>(result), "Whole-file read failed");
    return std::get<std::vector<std::uint8_t>>(std::move(result));
}

// Controlled stream for partial reads and otherwise hard-to-trigger failures.
// Exercising helpers against it keeps the checks independent of QFile internals.
class PartialStream final : public AssetFile {
public:
    PartialStream() : AssetFile("fixture.bin", "fixture.bin") {}
    std::int64_t at = 0, advertised = 6;
    bool ioFault = false, seekFault = false, sizeFault = false;
    Result<std::int64_t> size() override {
        if (sizeFault) return failure(ErrorCode::ioError, "size", "Injected metadata failure");
        return advertised;
    }
    Result<std::int64_t> position() override { return at; }
    Status seek(std::int64_t offset) override {
        if (seekFault) return failure(ErrorCode::ioError, "seek", "Injected seek failure");
        at = offset;
        return std::monostate{};
    }
    ReadResult read(void* destination, std::int64_t capacity) override {
        if (at >= 6) return {};
        const auto count = std::min<std::int64_t>({capacity, 2, 6 - at});
        auto* output = static_cast<std::uint8_t*>(destination);
        for (std::int64_t i = 0; i < count; ++i) output[i] = static_cast<std::uint8_t>(at + i);
        at += count;
        if (ioFault && at >= 4) return {count, failure(ErrorCode::ioError, "read", "Injected partial I/O failure")};
        return {count, {}};
    }
};

int main() {
    try {
        QTemporaryDir fixture;
        require(fixture.isValid(), "Cannot create fixture directory");
        const auto root = QFileInfo(fixture.path()).filesystemCanonicalFilePath();
        fs::create_directory(root / "Sounds");
        const std::vector<std::uint8_t> bytes = {0, 1, 13, 10, 26, 127, 128, 255, 0};
        const auto path = root / "Sounds/Bytes.bin";
        write(path, bytes);
        write(root / "Empty.bin", {});
        auto configured = AssetStore::create(root, {"C:/Game"});
        auto store = std::get<AssetStore>(std::move(configured));
        rejects(AssetStore::create(root / "missing"), ErrorCode::invalidRoot);
        rejects(store.open("missing"), ErrorCode::notFound);
        rejects(store.open("Sounds"), ErrorCode::notRegularFile);
        rejects(store.open("../outside"), ErrorCode::invalidPath);

        auto file = opened(store, "c:\\GAME\\sOuNdS\\BYTES.BIN");
        auto second = opened(store, "Sounds/Bytes.bin");
        require(number(file->size()) == 9 && number(file->position()) == 0, "Initial size/position");
        std::array<std::uint8_t, 16> buffer;
        buffer.fill(0xcc);
        auto read = file->read(buffer.data(), 3);
        require(!read.error && read.transferred == 3 && buffer[0] == 0 && buffer[2] == 13 && buffer[3] == 0xcc,
                "Binary read/count/untouched tail");
        require(number(file->position()) == 3 && number(second->position()) == 0, "Independent handle positions");
        require(number(file->size()) == 9 && number(file->position()) == 3, "Size changed position");
        rejects(file->seek(-1), ErrorCode::invalidArgument);
        rejects(file->seek(10), ErrorCode::invalidArgument);
        rejects(file->seek(std::numeric_limits<std::int64_t>::max()), ErrorCode::invalidArgument);
        require(number(file->position()) == 3, "Invalid seeks changed position");
        rejects(file->read(nullptr, 1), ErrorCode::invalidArgument);
        rejects(file->read(buffer.data(), -1), ErrorCode::invalidArgument);
        rejects(readExact(*file, nullptr, 1), ErrorCode::invalidArgument);
        rejects(readExact(*file, buffer.data(), -1), ErrorCode::invalidArgument);
        require(!file->read(nullptr, 0).error && !readExact(*file, nullptr, 0).error, "Zero reads reject null");
        require(number(file->position()) == 3, "Invalid/zero reads changed position");
        ok(file->seek(7));
        buffer.fill(0xcc);
        read = file->read(buffer.data(), buffer.size());
        require(!read.error && read.transferred == 2 && buffer[0] == 255 && buffer[1] == 0 && buffer[2] == 0xcc,
                "Short read at EOF");
        read = file->read(buffer.data() + 2, 1);
        require(!read.error && read.transferred == 0 && buffer[2] == 0xcc, "EOF mutated output");
        ok(file->seek(9));
        ok(file->seek(7));
        buffer.fill(0xcc);
        read = readExact(*file, buffer.data(), 4);
        rejects(read, ErrorCode::unexpectedEof, 2);
        require(buffer[0] == 255 && buffer[1] == 0 && buffer[2] == 0xcc && number(file->position()) == 9,
                "Failed exact read lost partial result");
        require(read.error->requestedPath == "c:\\GAME\\sOuNdS\\BYTES.BIN" && read.error->resolvedPath == path,
                "Missing file error context");

        ok(file->seek(2));
        rejects(readWhole(*file, -1), ErrorCode::invalidArgument);
        rejects(readWhole(*file, 8), ErrorCode::limitExceeded);
        require(number(file->position()) == 2, "Rejected whole read changed position");
        auto owned = whole(*file, 9);
        require(owned == bytes && number(file->position()) == 9, "Whole read did not restart at zero");
        file.reset();
        require(owned == bytes, "Owned buffer depends on closed file");
        auto empty = opened(store, "empty.BIN");
        require(number(empty->size()) == 0 && whole(*empty, 0).empty(), "Empty file read");
        ok(empty->seek(0));
        rejects(readExact(*empty, buffer.data(), 1), ErrorCode::unexpectedEof);

        std::vector<std::uint8_t> large(180000);
        for (std::size_t i = 0; i < large.size(); ++i) large[i] = static_cast<std::uint8_t>(i * 17);
        write(root / "Large.bin", large);
        auto big = opened(store, "large.bin");
        require(whole(*big, large.size()) == large, "Multi-read whole-file bytes differ");
        ok(big->seek(65530));
        buffer.fill(0xcc);
        read = readExact(*big, buffer.data(), buffer.size());
        require(!read.error && read.transferred == 16 &&
                std::equal(buffer.begin(), buffer.end(), large.begin() + 65530), "Seek read crosses chunk boundary");

        std::unique_ptr<AssetFile> survivor;
        {
            auto temporaryStore = std::get<AssetStore>(AssetStore::create(root));
            survivor = opened(temporaryStore, "Sounds/Bytes.bin");
        }
        require(whole(*survivor, 9) == bytes, "Handle depends on store lifetime");
        const auto sizeBefore = number(second->size());
        fs::resize_file(path, 4);
        require(number(second->size()) == 4, "Size was cached");
        buffer.fill(0xcc);
        rejects(readExact(*second, buffer.data(), sizeBefore), ErrorCode::unexpectedEof, 4);
        require(buffer[4] == 0xcc, "Truncation exposed stale buffered bytes");
        write(path, bytes);
        require(number(second->size()) == 9, "Growing file size was cached");
        require(whole(*second, 9) == bytes, "Read access altered source bytes");

        write(root / "Denied.bin", bytes);
        fs::permissions(root / "Denied.bin", fs::perms::none);
        QFile probe(root / "Denied.bin");
        const bool privileged = probe.open(QIODevice::ReadOnly);
        probe.close();
        const auto denied = store.open("Denied.bin");
        fs::permissions(root / "Denied.bin", fs::perms::owner_all);
        if (!privileged) rejects(denied, ErrorCode::permissionDenied);
        else std::cout << "Permission fixture skipped: process can read mode-000 file\n";

        PartialStream partial;
        buffer.fill(0xcc);
        read = readExact(partial, buffer.data(), 6);
        require(!read.error && read.transferred == 6 && buffer[5] == 5 && buffer[6] == 0xcc, "Partial reads not assembled");
        partial.at = 0;
        partial.ioFault = true;
        buffer.fill(0xcc);
        rejects(readExact(partial, buffer.data(), 6), ErrorCode::ioError, 4);
        require(partial.at == 4 && buffer[3] == 3 && buffer[4] == 0xcc, "I/O failure count/position not retained");
        rejects(readWhole(partial, 6), ErrorCode::ioError);
        partial.ioFault = false;
        partial.advertised = 9;
        rejects(readWhole(partial, 9), ErrorCode::unexpectedEof);
        partial.advertised = std::numeric_limits<std::int64_t>::max();
        rejects(readWhole(partial, 6), ErrorCode::limitExceeded);
        partial.advertised = 6;
        partial.seekFault = true;
        rejects(readWhole(partial, 6), ErrorCode::ioError);
        partial.seekFault = false;
        partial.sizeFault = true;
        rejects(readWhole(partial, 6), ErrorCode::ioError);

        std::cout << "Asset binary reads, seeks, EOF, limits, errors and ownership passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Asset file test failed: " << e.what() << '\n';
        return 1;
    }
}
