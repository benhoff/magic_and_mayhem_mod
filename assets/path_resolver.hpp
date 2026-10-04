#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace mnm::assets {
enum class ErrorCode {
    invalidRoot, invalidPath, unsupportedPath, notFound, ambiguousPath,
    outsideRoot, notDirectory, notRegularFile, permissionDenied,
    invalidArgument, unexpectedEof, limitExceeded, ioError
};
struct Error {
    ErrorCode code;
    std::string operation;
    std::string requestedPath;
    std::filesystem::path resolvedPath;
    std::string detail;
};
template<class T> using Result = std::variant<T, Error>;
struct ResolvedAsset {
    // Matched spelling, including in-root links, and canonical file target.
    std::filesystem::path matchedPath;
    std::filesystem::path canonicalPath;
};

// Read-only resolution; does not open or read the target file. No Qt types
// cross this boundary. Requires a trusted, stable installation during resolve.
class PathResolver {
public:
    static Result<PathResolver> create(const std::filesystem::path& root,
                                      const std::vector<std::string>& windowsPrefixes = {});
    Result<ResolvedAsset> resolve(std::string_view requestedPath) const;
    const std::filesystem::path& root() const { return root_; }
private:
    PathResolver(std::filesystem::path root, std::vector<std::vector<std::string>> prefixes);
    std::filesystem::path root_;
    std::vector<std::vector<std::string>> prefixes_;
};
}
