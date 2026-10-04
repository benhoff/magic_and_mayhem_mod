#pragma once
#include "path_resolver.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>

namespace mnm::assets {
struct ReadResult {
    std::int64_t transferred = 0;
    std::optional<Error> error;
};
using Status = Result<std::monostate>;

// Synchronous, thread-confined stream. Caller owns read destinations; each
// noncopyable handle owns its backend resource and closes it on destruction.
class AssetFile {
public:
    virtual ~AssetFile() = default;
    AssetFile(const AssetFile&) = delete;
    AssetFile& operator=(const AssetFile&) = delete;
    virtual Result<std::int64_t> size() = 0;
    virtual Result<std::int64_t> position() = 0;
    virtual Status seek(std::int64_t offset) = 0;
    virtual ReadResult read(void* destination, std::int64_t capacity) = 0;
protected:
    AssetFile(std::string request, std::filesystem::path resolved)
        : request_(std::move(request)), resolved_(std::move(resolved)) {}
    Error failure(ErrorCode code, std::string operation, std::string detail) const;
private:
    std::string request_;
    std::filesystem::path resolved_;
    friend ReadResult readExact(AssetFile&, void*, std::int64_t);
    friend Result<std::vector<std::uint8_t>> readWhole(AssetFile&, std::int64_t);
};

ReadResult readExact(AssetFile& file, void* destination, std::int64_t count);
Result<std::vector<std::uint8_t>> readWhole(AssetFile& file, std::int64_t limit);

class AssetStore {
public:
    static Result<AssetStore> create(const std::filesystem::path& root,
                                    const std::vector<std::string>& windowsPrefixes = {});
    Result<std::unique_ptr<AssetFile>> open(std::string_view requestedPath) const;
    const std::filesystem::path& root() const { return resolver_.root(); }
private:
    explicit AssetStore(PathResolver resolver) : resolver_(std::move(resolver)) {}
    PathResolver resolver_;
};
}
