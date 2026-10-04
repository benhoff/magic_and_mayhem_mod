#include "asset_file.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>

using namespace mnm::assets;
namespace fs = std::filesystem;
namespace {
constexpr std::int64_t Block = 32768;
QString hostPath(const fs::path& path) { return QFileInfo(path).filePath(); }
QString category(ErrorCode code) {
    switch (code) {
    case ErrorCode::invalidRoot: return "invalidRoot";
    case ErrorCode::invalidPath: return "invalidPath";
    case ErrorCode::unsupportedPath: return "unsupportedPath";
    case ErrorCode::notFound: return "notFound";
    case ErrorCode::ambiguousPath: return "ambiguousPath";
    case ErrorCode::outsideRoot: return "outsideRoot";
    case ErrorCode::notDirectory: return "notDirectory";
    case ErrorCode::notRegularFile: return "notRegularFile";
    case ErrorCode::permissionDenied: return "permissionDenied";
    case ErrorCode::invalidArgument: return "invalidArgument";
    case ErrorCode::unexpectedEof: return "unexpectedEof";
    case ErrorCode::limitExceeded: return "limitExceeded";
    case ErrorCode::ioError: return "ioError";
    }
    return "ioError";
}
struct Failure { Error error; };
template<class T> T take(Result<T> result) {
    if (const auto* e = std::get_if<Error>(&result)) throw Failure{*e};
    return std::get<T>(std::move(result));
}
void checked(const ReadResult& read) { if (read.error) throw Failure{*read.error}; }
Error referenceFailure(const fs::path& path, const std::string& detail) {
    std::error_code ec;
    const auto status = fs::status(path, ec);
    const auto code = ec == std::errc::permission_denied ? ErrorCode::permissionDenied :
        (!ec && !fs::exists(status)) || ec == std::errc::no_such_file_or_directory ? ErrorCode::notFound : ErrorCode::ioError;
    return {code, "referenceRead", {}, path, detail};
}
struct Snapshot { std::int64_t size; QString hash; };
Snapshot snapshot(const fs::path& path) {
    std::error_code ec;
    if (!fs::is_regular_file(path, ec)) throw Failure{referenceFailure(path, "Reference is missing or not a regular file")};
    std::ifstream input(path, std::ios::binary);
    if (!input) throw Failure{referenceFailure(path, "Cannot open independent binary reference")};
    QCryptographicHash digest(QCryptographicHash::Sha256);
    std::array<char, Block> buffer;
    std::int64_t total = 0;
    while (input) {
        input.read(buffer.data(), buffer.size());
        const auto n = input.gcount();
        if (n > std::numeric_limits<std::int64_t>::max() - total)
            throw Failure{referenceFailure(path, "Reference exceeds signed 64-bit size")};
        total += n;
        digest.addData(QByteArrayView(buffer.data(), n));
    }
    if (input.bad() || !input.eof()) throw Failure{referenceFailure(path, "Independent binary reference read failed")};
    return {total, QString::fromLatin1(digest.result().toHex())};
}
void referenceRead(std::ifstream& input, char* buffer, std::int64_t n, const fs::path& path) {
    if (!n) return;
    input.read(buffer, n);
    if (input.gcount() != n || input.bad()) throw Failure{referenceFailure(path, "Short or failed independent reference read")};
}
QJsonObject compare(const AssetStore& store, const PathResolver& resolver, const QJsonObject& entry) {
    const auto request = entry.value("path").toString();
    const auto reference = QFileInfo(entry.value("reference").toString()).filesystemAbsoluteFilePath();
    QJsonObject row{{"path", request}, {"reference", hostPath(reference)}, {"status", "error"}};
    try {
        const auto resolved = take(resolver.resolve(request.toStdString()));
        row.insert("resolved", hostPath(resolved.canonicalPath));
        const auto sourceBefore = snapshot(resolved.canonicalPath);
        const auto referenceBefore = snapshot(reference);
        row.insert("source_sha256_before", sourceBefore.hash);
        row.insert("reference_sha256_before", referenceBefore.hash);
        auto file = take(store.open(request.toStdString()));
        const auto size = take(file->size());
        row.insert("size", qint64(size));
        row.insert("reference_size", qint64(referenceBefore.size));
        row.insert("size_equal", size == referenceBefore.size);
        std::ifstream input(reference, std::ios::binary);
        if (!input) throw Failure{referenceFailure(reference, "Cannot reopen reference")};
        std::array<char, Block> actual{}, expected{};
        std::optional<std::int64_t> first;
        auto differences = [&](std::int64_t offset, std::int64_t a, std::int64_t b) {
            const auto common = std::min(a, b);
            for (std::int64_t i = 0; i < common; ++i) if (actual[i] != expected[i]) {
                const auto at = offset + i;
                if (!first || at < *first) first = at;
                break;
            }
            if (a != b && (!first || offset + common < *first)) first = offset + common;
        };
        QCryptographicHash sequential(QCryptographicHash::Sha256);
        const auto extent = std::max(size, referenceBefore.size);
        for (std::int64_t offset = 0; offset < extent;) {
            const auto width = std::min(Block, extent - offset);
            const auto a = std::max<std::int64_t>(0, std::min(width, size - offset));
            const auto b = std::max<std::int64_t>(0, std::min(width, referenceBefore.size - offset));
            checked(readExact(*file, actual.data(), a));
            referenceRead(input, expected.data(), b, reference);
            sequential.addData(QByteArrayView(actual.data(), a));
            differences(offset, a, b);
            offset += width;
        }
        const auto sequentialHash = QString::fromLatin1(sequential.result().toHex());
        row.insert("sequential_sha256", sequentialHash);
        row.insert("sequential_complete", true);
        // Visit every block backwards, explicitly seeking before each read.
        // This compares all bytes again without reusing the sequential buffer.
        bool seekEqual = size == referenceBefore.size;
        std::int64_t seeks = 0;
        for (std::int64_t end = extent; end > 0;) {
            const auto offset = std::max<std::int64_t>(0, end - Block);
            const auto a = std::max<std::int64_t>(0, std::min(end - offset, size - offset));
            const auto b = std::max<std::int64_t>(0, std::min(end - offset, referenceBefore.size - offset));
            if (a) { take(file->seek(offset)); checked(readExact(*file, actual.data(), a)); ++seeks; }
            if (b) {
                input.clear(); input.seekg(offset);
                if (!input) throw Failure{referenceFailure(reference, "Reference seek failed")};
                referenceRead(input, expected.data(), b, reference);
            }
            if (a != b || !std::equal(actual.begin(), actual.begin() + std::min(a, b), expected.begin())) seekEqual = false;
            differences(offset, a, b);
            end = offset;
        }
        take(file->seek(size));
        char tail;
        const auto eof = file->read(&tail, 1);
        checked(eof);
        const auto sourceAfter = snapshot(resolved.canonicalPath);
        const auto referenceAfter = snapshot(reference);
        row.insert("source_sha256_after", sourceAfter.hash);
        row.insert("reference_sha256_after", referenceAfter.hash);
        row.insert("seek_reads", qint64(seeks));
        row.insert("seek_complete", true);
        row.insert("seek_equal", seekEqual);
        row.insert("first_differing_offset", first ? QJsonValue(qint64(*first)) : QJsonValue(QJsonValue::Null));
        const bool unchanged = sourceBefore.size == sourceAfter.size && sourceBefore.hash == sourceAfter.hash &&
            referenceBefore.size == referenceAfter.size && referenceBefore.hash == referenceAfter.hash;
        const bool sourceConsistent = size == sourceBefore.size && sequentialHash == sourceBefore.hash && eof.transferred == 0;
        row.insert("inputs_unchanged", unchanged);
        row.insert("interface_matches_source", sourceConsistent);
        row.insert("status", !unchanged ? "changed" : first || !seekEqual || !sourceConsistent ? "different" : "equal");
    } catch (const Failure& f) {
        row.insert("error", QJsonObject{{"category", category(f.error.code)},
            {"operation", QString::fromStdString(f.error.operation)},
            {"requested_path", QString::fromStdString(f.error.requestedPath)},
            {"resolved_path", hostPath(f.error.resolvedPath)}, {"detail", QString::fromStdString(f.error.detail)}});
    }
    return row;
}
}

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Compare raw asset-interface reads with independent binary reference reads.");
    parser.addHelpOption();
    parser.addOption({"root", "Installed asset root.", "directory"});
    parser.addOption({"prefix", "Windows installation prefix (repeatable).", "prefix"});
    parser.addOption({"manifest", "JSON array of path/reference pairs (host references must be absolute).", "file"});
    parser.addOption({"report", "Write JSON report atomically; default stdout.", "file"});
    parser.process(app);
    if (!parser.isSet("root") || !parser.isSet("manifest")) parser.showHelp(2);
    try {
        QFile manifest(parser.value("manifest"));
        if (!manifest.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot open comparison manifest");
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(manifest.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isArray() || document.array().isEmpty())
            throw std::runtime_error("Manifest must be a nonempty JSON array");
        for (const auto& value : document.array()) {
            const auto entry = value.toObject();
            if (!value.isObject() || !entry.value("path").isString() || !entry.value("reference").isString() ||
                entry.value("path").toString().isEmpty() || entry.value("reference").toString().isEmpty() ||
                !QFileInfo(entry.value("reference").toString()).isAbsolute() || entry.value("reference").toString().contains(QChar(0)))
                throw std::runtime_error("Each manifest entry requires a nonempty path and absolute reference");
        }
        std::vector<std::string> prefixes;
        for (const auto& prefix : parser.values("prefix")) prefixes.push_back(prefix.toStdString());
        const auto root = QFileInfo(parser.value("root")).filesystemAbsoluteFilePath();
        const auto store = take(AssetStore::create(root, prefixes));
        const auto resolver = take(PathResolver::create(root, prefixes));
        // Reports must not overwrite installation files, references or inputs.
        if (parser.isSet("report")) {
            const auto output = fs::weakly_canonical(QFileInfo(parser.value("report")).filesystemAbsoluteFilePath());
            const auto relative = output.lexically_relative(store.root());
            if (!relative.empty() && *relative.begin() != "..") throw std::runtime_error("Report must be outside the installation root");
            if (output == fs::canonical(QFileInfo(parser.value("manifest")).filesystemAbsoluteFilePath()))
                throw std::runtime_error("Report would overwrite manifest");
            for (const auto& entry : document.array()) {
                const auto reference = QFileInfo(entry.toObject().value("reference").toString()).filesystemAbsoluteFilePath();
                if (output == fs::weakly_canonical(reference)) throw std::runtime_error("Report would overwrite a reference");
            }
        }
        QJsonArray rows;
        int equal = 0, different = 0, errors = 0, changed = 0;
        for (const auto& entry : document.array()) {
            const auto row = compare(store, resolver, entry.toObject());
            const auto status = row.value("status").toString();
            if (status == "equal") ++equal;
            else if (status == "different") ++different;
            else if (status == "changed") ++changed;
            else ++errors;
            rows.append(row);
        }
        const QJsonObject report{{"schema_version", 1}, {"origin", "raw_asset_interface_comparison"},
            {"root", hostPath(store.root())}, {"qt_version", qVersion()},
            {"equal", equal}, {"different", different}, {"errors", errors}, {"changed", changed}, {"assets", rows}};
        const auto bytes = QJsonDocument(report).toJson();
        if (parser.isSet("report")) {
            QSaveFile output(parser.value("report"));
            if (!output.open(QIODevice::WriteOnly) || output.write(bytes) != bytes.size() || !output.commit())
                throw std::runtime_error("Cannot write report");
        } else std::cout << bytes.constData();
        return errors || changed ? 2 : different ? 1 : 0;
    } catch (const Failure& f) {
        std::cerr << category(f.error.code).toStdString() << ": " << f.error.detail << '\n';
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; }
    return 2;
}
