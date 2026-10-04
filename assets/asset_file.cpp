#include "asset_file.hpp"
#include <QFile>
#include <algorithm>
#include <cerrno>
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
bool validCount(std::int64_t count) {
    return count >= 0 && static_cast<std::uint64_t>(count) <= std::numeric_limits<std::size_t>::max() &&
        static_cast<std::uint64_t>(count) <= static_cast<std::uint64_t>(std::numeric_limits<std::ptrdiff_t>::max());
}

class QtAssetFile final : public AssetFile {
public:
    QtAssetFile(std::string request, const std::filesystem::path& path)
        : AssetFile(std::move(request), path), file_(path) {}
    Status open() {
        // No text conversion, write access, or read-ahead hiding truncation.
        errno = 0;
        if (!file_.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
            // Qt can report OpenError for permission failures. Capture errno
            // immediately; never infer categories from translated error text.
            const auto nativeError = errno;
            auto e = qtFailure("open");
            if (nativeError == EACCES || nativeError == EPERM) e.code = ErrorCode::permissionDenied;
            else if (nativeError == ENOENT) e.code = ErrorCode::notFound;
            else if (nativeError == ENOTDIR) e.code = ErrorCode::notDirectory;
            else if (nativeError == ENOMEM || nativeError == EMFILE || nativeError == ENFILE)
                e.code = ErrorCode::limitExceeded;
            return e;
        }
        return std::monostate{};
    }
    Result<std::int64_t> size() override {
        file_.unsetError();
        const auto value = file_.size();
        if (value < 0 || file_.error() != QFileDevice::NoError) return qtFailure("size");
        return static_cast<std::int64_t>(value);
    }
    Result<std::int64_t> position() override {
        file_.unsetError();
        const auto value = file_.pos();
        if (value < 0 || file_.error() != QFileDevice::NoError) return qtFailure("position");
        return static_cast<std::int64_t>(value);
    }
    Status seek(std::int64_t offset) override {
        if (offset < 0) return failure(ErrorCode::invalidArgument, "seek", "Negative offset");
        const auto length = size();
        if (const auto* e = std::get_if<Error>(&length)) return *e;
        if (offset > std::get<std::int64_t>(length))
            return failure(ErrorCode::invalidArgument, "seek", "Offset exceeds file size");
        file_.unsetError();
        if (!file_.seek(offset)) return qtFailure("seek");
        return std::monostate{};
    }
    ReadResult read(void* destination, std::int64_t capacity) override {
        if (!validCount(capacity) || (capacity && !destination))
            return {0, failure(ErrorCode::invalidArgument, "read", "Invalid destination or capacity")};
        if (!capacity) return {};
        file_.unsetError();
        // Bounded requests permit callers to stream large files without Qt
        // internal allocations proportional to the entire requested capacity.
        const auto transferred = file_.read(static_cast<char*>(destination), std::min<std::int64_t>(capacity, 65536));
        if (transferred < 0) return {0, qtFailure("read")};
        if (file_.error() != QFileDevice::NoError) return {transferred, qtFailure("read")};
        return {transferred, {}};
    }
private:
    Error qtFailure(const std::string& operation) const {
        const auto code = file_.error() == QFileDevice::PermissionsError ? ErrorCode::permissionDenied :
            file_.error() == QFileDevice::ResourceError ? ErrorCode::limitExceeded : ErrorCode::ioError;
        return failure(code, operation, file_.errorString().toStdString());
    }
    QFile file_;
};
}

Error AssetFile::failure(ErrorCode code, std::string operation, std::string detail) const {
    return {code, std::move(operation), request_, resolved_, std::move(detail)};
}

ReadResult readExact(AssetFile& file, void* destination, std::int64_t count) {
    if (!validCount(count) || (count && !destination))
        return {0, file.failure(ErrorCode::invalidArgument, "readExact", "Invalid destination or count")};
    std::int64_t total = 0;
    while (total < count) {
        const auto step = file.read(static_cast<std::uint8_t*>(destination) + static_cast<std::size_t>(total), count - total);
        if (step.transferred < 0 || step.transferred > count - total)
            return {total, file.failure(ErrorCode::ioError, "readExact", "Backend returned an invalid byte count")};
        total += step.transferred;
        if (step.error) return {total, step.error};
        if (!step.transferred)
            return {total, file.failure(ErrorCode::unexpectedEof, "readExact", "EOF before requested count")};
    }
    return {total, {}};
}

Result<std::vector<std::uint8_t>> readWhole(AssetFile& file, std::int64_t limit) {
    if (limit < 0) return file.failure(ErrorCode::invalidArgument, "readWhole", "Negative size limit");
    const auto length = file.size();
    if (const auto* e = std::get_if<Error>(&length)) return *e;
    const auto count = std::get<std::int64_t>(length);
    if (count < 0) return file.failure(ErrorCode::ioError, "readWhole", "Backend returned a negative size");
    if (count > limit || !validCount(count))
        return file.failure(ErrorCode::limitExceeded, "readWhole", "File exceeds size or allocation limit");
    try {
        std::vector<std::uint8_t> bytes;
        if (static_cast<std::uint64_t>(count) > bytes.max_size())
            return file.failure(ErrorCode::limitExceeded, "readWhole", "File exceeds vector capacity");
        bytes.resize(static_cast<std::size_t>(count));
        const auto seek = file.seek(0);
        if (const auto* e = std::get_if<Error>(&seek)) return *e;
        const auto read = readExact(file, bytes.data(), count);
        if (read.error) return *read.error;
        return bytes;
    } catch (const std::bad_alloc&) {
        return file.failure(ErrorCode::limitExceeded, "readWhole", "Allocation failed");
    } catch (const std::length_error&) {
        return file.failure(ErrorCode::limitExceeded, "readWhole", "Allocation size is unsupported");
    }
}

Result<AssetStore> AssetStore::create(const std::filesystem::path& root,
                                     const std::vector<std::string>& windowsPrefixes) {
    auto configured = PathResolver::create(root, windowsPrefixes);
    if (const auto* e = std::get_if<Error>(&configured)) return *e;
    return AssetStore(std::get<PathResolver>(std::move(configured)));
}

Result<std::unique_ptr<AssetFile>> AssetStore::open(std::string_view requestedPath) const {
    auto resolved = resolver_.resolve(requestedPath);
    if (const auto* e = std::get_if<Error>(&resolved)) return *e;
    const auto& path = std::get<ResolvedAsset>(resolved).canonicalPath;
    try {
        auto handle = std::make_unique<QtAssetFile>(std::string(requestedPath), path);
        const auto opened = handle->open();
        if (const auto* e = std::get_if<Error>(&opened)) return *e;
        return std::unique_ptr<AssetFile>(std::move(handle));
    } catch (const std::bad_alloc&) {
        return Error{ErrorCode::limitExceeded, "open", std::string(requestedPath), path, "Handle allocation failed"};
    }
}
}
