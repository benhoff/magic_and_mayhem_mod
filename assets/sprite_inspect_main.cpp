#include "sprite_loader.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace mnm::assets;
namespace {
QString errorName(SpriteErrorCode code) {
    switch (code) {
    case SpriteErrorCode::invalidArgument: return "invalidArgument";
    case SpriteErrorCode::invalidFormat: return "invalidFormat";
    case SpriteErrorCode::unsupportedVersion: return "unsupportedVersion";
    case SpriteErrorCode::malformedData: return "malformedData";
    case SpriteErrorCode::limitExceeded: return "limitExceeded";
    case SpriteErrorCode::assetInput: return "assetInput";
    }
    return "assetInput";
}
QByteArray packedPixels(const SpriteFrame& frame) {
    if (const auto* bytes = std::get_if<std::vector<std::uint8_t>>(&frame.pixels))
        return QByteArray(reinterpret_cast<const char*>(bytes->data()), static_cast<qsizetype>(bytes->size()));
    const auto& words = std::get<std::vector<std::uint16_t>>(frame.pixels);
    QByteArray bytes;
    bytes.reserve(static_cast<qsizetype>(words.size() * 2));
    for (const auto value : words) { bytes.append(char(value & 255)); bytes.append(char(value >> 8)); }
    return bytes;
}
QString hash(const QByteArray& bytes) { return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex(); }
QJsonObject failure(const QString& path, const SpriteError& error) {
    QJsonObject row{{"path", path}, {"status", "error"}, {"code", errorName(error.code)},
                    {"offset", qint64(error.offset)}, {"detail", QString::fromStdString(error.detail)}};
    if (error.frame) row["frame"] = qint64(*error.frame);
    if (error.input) row["input"] = QJsonObject{{"code", int(error.input->code)},
        {"operation", QString::fromStdString(error.input->operation)},
        {"requested_path", QString::fromStdString(error.input->requestedPath)},
        {"resolved_path", QString::fromStdString(error.input->resolvedPath.string())},
        {"detail", QString::fromStdString(error.input->detail)}};
    return row;
}
}
int main(int argc, char** argv) try {
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription("Read version-4 SPR files through AssetFile; JSON inspection only");
    parser.addHelpOption();
    parser.addOption({"root", "Installed asset root", "directory"});
    parser.addOption({"prefix", "Explicit Windows installation alias (repeatable)", "path"});
    parser.addOption({"manifest", "JSON array of {path, frames:[indices]}", "file"});
    parser.addOption({"path", "One asset request instead of a manifest", "path"});
    parser.addOption({"frame", "Frame index for --path (repeatable; default 0)", "index"});
    parser.process(app);
    if (!parser.isSet("root") || parser.isSet("manifest") == parser.isSet("path"))
        throw std::runtime_error("Specify --root and exactly one of --path or --manifest");
    if (parser.isSet("manifest") && parser.isSet("frame")) throw std::runtime_error("--frame requires --path");
    std::vector<std::string> prefixes;
    for (const auto& value : parser.values("prefix")) prefixes.push_back(value.toStdString());
    auto configured = AssetStore::create(parser.value("root").toStdString(), prefixes);
    if (const auto* error = std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store = std::get<AssetStore>(std::move(configured));
    QJsonArray entries;
    if (parser.isSet("manifest")) {
        QFile input(parser.value("manifest"));
        if (!input.open(QIODevice::ReadOnly) || input.size() > 4 * 1024 * 1024)
            throw std::runtime_error("Cannot read manifest within 4 MiB limit");
        QJsonParseError error;
        auto document = QJsonDocument::fromJson(input.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !document.isArray())
            throw std::runtime_error("Expected JSON manifest array");
        entries = document.array();
    } else {
        QJsonArray indices;
        for (const auto& value : parser.values("frame")) {
            bool ok = false; const auto index = value.toULongLong(&ok);
            if (!ok || index > std::numeric_limits<std::uint32_t>::max()) throw std::runtime_error("Invalid frame index");
            indices.append(qint64(index));
        }
        if (indices.empty()) indices.append(0);
        entries.append(QJsonObject{{"path", parser.value("path")}, {"frames", indices}});
    }
    if (entries.empty() || entries.size() > 4096) throw std::runtime_error("Manifest must contain 1..4096 entries");
    QJsonArray rows;
    bool errors = false;
    std::uint64_t sampleBytes = 0;
    unsigned sampleCount = 0;
    for (const auto& value : entries) {
        if (!value.isObject()) throw std::runtime_error("Manifest entry must be an object");
        const auto entry = value.toObject();
        if (!entry.value("path").isString() || !entry.value("frames").isArray())
            throw std::runtime_error("Each entry requires a path and frame-index array");
        const auto path = entry.value("path").toString();
        auto opened = store.open(path.toStdString());
        if (const auto* error = std::get_if<Error>(&opened)) {
            rows.append(failure(path, {SpriteErrorCode::assetInput, 0, {}, error->detail, *error}));
            errors = true; continue;
        }
        auto handle = std::get<std::unique_ptr<AssetFile>>(std::move(opened));
        auto result = loadSprite(*handle);
        handle.reset(); // Inspect only owned decoded output after closing input.
        if (const auto* error = std::get_if<SpriteError>(&result)) {
            rows.append(failure(path, *error)); errors = true; continue;
        }
        const auto& sprite = std::get<Sprite>(result);
        QJsonArray palettes, frames;
        for (const auto& palette : sprite.palettes) {
            QByteArray bytes;
            for (const auto& colour : palette) { bytes.append(char(colour.red)); bytes.append(char(colour.green)); bytes.append(char(colour.blue)); }
            palettes.append(QString(bytes.toHex()));
        }
        std::uint64_t totalPixels = 0, emptyFrames = 0;
        for (const auto& frame : sprite.frames) { totalPixels += std::uint64_t(frame.width) * frame.height; emptyFrames += frame.empty(); }
        for (const auto& requested : entry.value("frames").toArray()) {
            const auto n = requested.toDouble(-1);
            if (!requested.isDouble() || n < 0 || n >= sprite.frames.size() || n != std::uint32_t(n))
                throw std::runtime_error("Requested frame is not a valid integer index");
            const auto index = static_cast<std::uint32_t>(n);
            const auto& frame = sprite.frames[index];
            const auto footprint = frame.opaqueMask.size() * (sprite.storage == SpriteStorage::indexed8 ? 2ULL : 3ULL);
            if (++sampleCount > 1024 || footprint > 16ULL * 1024 * 1024 - sampleBytes)
                throw std::runtime_error("Inspection exceeds 1024 samples or 16 MiB raw sample output");
            sampleBytes += footprint;
            const auto pixels = packedPixels(frame);
            const QByteArray mask(reinterpret_cast<const char*>(frame.opaqueMask.data()), static_cast<qsizetype>(frame.opaqueMask.size()));
            const QByteArray name(reinterpret_cast<const char*>(frame.name.data()), 8);
            QJsonObject item{{"index", qint64(index)}, {"width", qint64(frame.width)}, {"height", qint64(frame.height)},
                {"origin_x", frame.originX}, {"origin_y", frame.originY}, {"name_hex", QString(name.toHex())},
                {"source_offset", qint64(frame.sourceOffset)}, {"encoded_size", qint64(frame.encodedSize)},
                {"auxiliary_offsets", QJsonArray{qint64(frame.auxiliaryOffsets[0]), qint64(frame.auxiliaryOffsets[1])}},
                {"palette_index", frame.paletteIndex ? QJsonValue(qint64(*frame.paletteIndex)) : QJsonValue(QJsonValue::Null)},
                {"pixels_sha256", hash(pixels)}, {"mask_sha256", hash(mask)},
                {"pixels_hex", QString(pixels.toHex())}, {"mask_hex", QString(mask.toHex())}};
            frames.append(item);
        }
        rows.append(QJsonObject{{"path", path}, {"status", "decoded"}, {"version", qint64(sprite.version)},
            {"header_flags", qint64(sprite.headerFlags)}, {"source_bytes", qint64(sprite.sourceBytes)},
            {"storage", sprite.storage == SpriteStorage::indexed8 ? "indexed8" : "rgb565"},
            {"frame_count", qint64(sprite.frames.size())}, {"empty_frames", qint64(emptyFrames)},
            {"pixels", qint64(totalPixels)}, {"palettes_rgb_hex", palettes}, {"frames", frames}});
    }
    std::cout << QJsonDocument(QJsonObject{{"files", rows}}).toJson(QJsonDocument::Compact).constData() << '\n';
    return errors ? 1 : 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 2; }
