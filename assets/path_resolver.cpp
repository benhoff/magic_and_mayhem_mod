#include "path_resolver.hpp"
#include <QFileInfo>
#include <QString>
#include <algorithm>
#include <system_error>
#include <utility>

namespace mnm::assets {
namespace {
std::string fold(std::string value) {
    for (auto& c : value) if (c >= 'A' && c <= 'Z') c = char(c + ('a' - 'A'));
    return value;
}
bool ascii(std::string_view value) {
    return std::all_of(value.begin(), value.end(), [](unsigned char c) { return c >= 32 && c < 127; });
}
bool letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool drive(const std::string& path) { return path.size() >= 2 && letter(path[0]) && path[1] == ':'; }
std::vector<std::string> split(const std::string& path) {
    std::vector<std::string> parts;
    for (std::size_t at = 0; at < path.size();) {
        const auto end = path.find('/', at);
        const auto part = path.substr(at, end == std::string::npos ? end : end - at);
        if (!part.empty()) parts.push_back(part);
        if (end == std::string::npos) break;
        at = end + 1;
    }
    return parts;
}
bool validName(const std::string& name) {
    if (name.empty() || name.back() == '.' || name.back() == ' ' ||
        name.find_first_of(":<>\"|?*") != std::string::npos) return false;
    auto stem = fold(name.substr(0, name.find('.')));
    // Windows also reserves device basenames with spaces before an extension.
    while (!stem.empty() && stem.back() == ' ') stem.pop_back();
    if (stem == "con" || stem == "prn" || stem == "aux" || stem == "nul") return false;
    return !(stem.size() == 4 && (stem.substr(0, 3) == "com" || stem.substr(0, 3) == "lpt") &&
             stem[3] >= '1' && stem[3] <= '9');
}
bool prefix(const std::vector<std::string>& head, const std::vector<std::string>& path) {
    return head.size() <= path.size() && std::equal(head.begin(), head.end(), path.begin(),
        [](const auto& a, const auto& b) { return fold(a) == fold(b); });
}
bool contained(const std::filesystem::path& root, const std::filesystem::path& path) {
    auto r = root.begin(), p = path.begin();
    for (; r != root.end(); ++r, ++p) if (p == path.end() || *r != *p) return false;
    return true;
}
Error error(ErrorCode code, std::string operation, std::string_view request,
            const std::filesystem::path& path, std::string detail) {
    return {code, std::move(operation), std::string(request), path, std::move(detail)};
}
Error filesystemError(std::string operation, std::string_view request,
                      const std::filesystem::path& path, const std::error_code& ec) {
    const auto code = ec == std::errc::permission_denied ? ErrorCode::permissionDenied :
        ec == std::errc::no_such_file_or_directory ? ErrorCode::notFound :
        ec == std::errc::not_a_directory ? ErrorCode::notDirectory : ErrorCode::ioError;
    return error(code, std::move(operation), request, path, ec.message());
}
}

PathResolver::PathResolver(std::filesystem::path root, std::vector<std::vector<std::string>> prefixes)
    : root_(std::move(root)), prefixes_(std::move(prefixes)) {}

Result<PathResolver> PathResolver::create(const std::filesystem::path& root,
                                         const std::vector<std::string>& windowsPrefixes) {
    if (root.empty()) return error(ErrorCode::invalidRoot, "configure", {}, root, "Empty installation root");
    std::error_code ec;
    const auto absolute = QFileInfo(root).filesystemAbsoluteFilePath();
    const auto canonical = std::filesystem::canonical(absolute, ec);
    if (ec) {
        auto failure = filesystemError("configure", {}, root, ec);
        if (failure.code == ErrorCode::notFound || failure.code == ErrorCode::notDirectory)
            failure.code = ErrorCode::invalidRoot;
        return failure;
    }
    const auto status = std::filesystem::status(canonical, ec);
    if (ec) return filesystemError("configure", {}, canonical, ec);
    if (!std::filesystem::is_directory(status))
        return error(ErrorCode::invalidRoot, "configure", {}, canonical, "Root is not a directory");
    std::vector<std::vector<std::string>> prefixes;
    for (auto alias : windowsPrefixes) {
        const auto original = alias;
        std::replace(alias.begin(), alias.end(), '\\', '/');
        if (!ascii(alias) || !drive(alias) || alias.size() < 3 || alias[2] != '/')
            return error(ErrorCode::invalidArgument, "configure", original, {}, "Alias must be an ASCII drive-absolute prefix");
        auto parts = split(alias);
        for (std::size_t i = 1; i < parts.size(); ++i)
            if (!validName(parts[i]))
                return error(ErrorCode::invalidArgument, "configure", original, {}, "Invalid alias component");
        for (const auto& existing : prefixes)
            if (prefix(existing, parts) || prefix(parts, existing))
                return error(ErrorCode::invalidArgument, "configure", original, {}, "Overlapping installation aliases");
        prefixes.push_back(std::move(parts));
    }
    return PathResolver(canonical, std::move(prefixes));
}

Result<ResolvedAsset> PathResolver::resolve(std::string_view requestedPath) const {
    auto fail = [&](ErrorCode code, const std::string& detail) {
        return error(code, "resolve", requestedPath, {}, detail);
    };
    if (requestedPath.empty() || !ascii(requestedPath))
        return fail(ErrorCode::invalidPath, "Request must be nonempty printable ASCII");
    std::string path(requestedPath);
    std::replace(path.begin(), path.end(), '\\', '/');
    auto parts = split(path);
    if (std::find(parts.begin(), parts.end(), "..") != parts.end())
        return fail(ErrorCode::invalidPath, "Parent traversal is unsupported");
    if (path.front() == '/') return fail(ErrorCode::unsupportedPath, "Root-relative, UNC and device paths are unsupported");
    parts.erase(std::remove(parts.begin(), parts.end(), "."), parts.end());
    if (drive(path)) {
        if (path.size() < 3 || path[2] != '/')
            return fail(ErrorCode::unsupportedPath, "Drive-relative paths are unsupported");
        bool mapped = false;
        for (const auto& alias : prefixes_) if (prefix(alias, parts)) {
            parts.erase(parts.begin(), parts.begin() + alias.size());
            mapped = true;
            break;
        }
        if (!mapped) return fail(ErrorCode::unsupportedPath, "No installation alias matches request");
    }
    if (parts.empty() || path.back() == '/')
        return fail(ErrorCode::invalidPath, "Request must identify a file");
    for (const auto& part : parts) if (!validName(part))
        return fail(ErrorCode::invalidPath, "Invalid or reserved name component");

    auto matched = root_;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        std::error_code ec;
        // Qt directory listings lack an enumeration error result. Use checked
        // C++ iteration so permission/I/O failures cannot appear as missing files.
        std::filesystem::directory_iterator it(matched, ec), end;
        if (ec) return filesystemError("enumerate", requestedPath, matched, ec);
        std::filesystem::path match;
        std::size_t count = 0;
        for (; it != end; it.increment(ec)) {
            if (ec) break;
            const auto name = QFileInfo(it->path()).fileName().toUtf8().toStdString();
            if (ascii(name) && fold(name) == fold(parts[i])) { match = it->path(); ++count; }
        }
        if (ec) return filesystemError("enumerate", requestedPath, matched, ec);
        if (count == 0) return error(ErrorCode::notFound, "resolve", requestedPath, matched, "No matching component: " + parts[i]);
        if (count > 1) return error(ErrorCode::ambiguousPath, "resolve", requestedPath, matched, "Multiple case-insensitive matches: " + parts[i]);
        matched = match;
        const auto canonical = std::filesystem::canonical(matched, ec);
        if (ec) return filesystemError("canonicalize", requestedPath, matched, ec);
        if (!contained(root_, canonical))
            return error(ErrorCode::outsideRoot, "resolve", requestedPath, canonical, "Target escapes installation root");
        const auto status = std::filesystem::status(canonical, ec);
        if (ec) return filesystemError("stat", requestedPath, canonical, ec);
        if (i + 1 < parts.size()) {
            if (!std::filesystem::is_directory(status))
                return error(ErrorCode::notDirectory, "resolve", requestedPath, matched, "Intermediate component is not a directory");
        } else {
            if (!std::filesystem::is_regular_file(status))
                return error(ErrorCode::notRegularFile, "resolve", requestedPath, matched, "Target is not a regular file");
            return ResolvedAsset{matched, canonical};
        }
    }
    return fail(ErrorCode::invalidPath, "Request has no file component");
}
}
