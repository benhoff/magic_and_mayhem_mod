#include "path_resolver.hpp"
#include <QTemporaryDir>
#include <QFileInfo>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <variant>

namespace fs = std::filesystem;
using namespace mnm::assets;
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void file(const fs::path& path) {
    std::ofstream out(path, std::ios::binary);
    out << "fixture";
    require(bool(out), "Cannot create fixture");
}
template<class T> void rejects(const Result<T>& result, ErrorCode expected) {
    const auto* failure = std::get_if<Error>(&result);
    require(failure && failure->code == expected, "Wrong error category");
    require(!failure->operation.empty() && !failure->detail.empty(), "Missing diagnostic");
}
void resolves(const PathResolver& resolver, const std::string& request,
              const fs::path& matched, const fs::path& canonical) {
    const auto result = resolver.resolve(request);
    const auto* value = std::get_if<ResolvedAsset>(&result);
    require(value && value->matchedPath == matched && value->canonicalPath == canonical,
            "Unexpected resolution: " + request);
}

int main() {
    try {
        QTemporaryDir fixture;
        require(fixture.isValid(), "Cannot create temporary directory");
        const auto base = QFileInfo(fixture.path()).filesystemCanonicalFilePath();
        const auto root = base / "install";
        fs::create_directories(root / "Sounds" / "Nested");
        const auto wav = root / "Sounds" / "Spell click.wav";
        file(wav);
        file(root / "Sounds" / "Nested" / "Tone.wav");
        file(root / ".Hidden");
        file(root / "Sounds" / fs::u8path("caf\xc3\xa9.wav"));
        auto configured = PathResolver::create(root, {"C:\\MagicMayhem", "D:/Games/MagicMayhem/"});
        require(std::holds_alternative<PathResolver>(configured), "Configuration failed");
        const auto resolver = std::get<PathResolver>(std::move(configured));
        resolves(resolver, "sOuNdS\\SPELL CLICK.WAV", wav, wav);
        resolves(resolver, "./Sounds//./Spell click.wav", wav, wav);
        resolves(resolver, "c:\\magicmayhem\\SOUNDS\\Spell click.wav", wav, wav);
        resolves(resolver, "D:/Games//MagicMayhem/./Sounds/Spell click.wav", wav, wav);
        resolves(resolver, "sounds/nEsTeD/tOnE.WaV", root / "Sounds/Nested/Tone.wav", root / "Sounds/Nested/Tone.wav");
        resolves(resolver, ".hidden", root / ".Hidden", root / ".Hidden");
        const auto unicodeRoot = base / fs::u8path("install-\xc3\xa9");
        fs::create_directory(unicodeRoot);
        file(unicodeRoot / "Asset.bin");
        const auto unicodeResolver = std::get<PathResolver>(PathResolver::create(unicodeRoot));
        resolves(unicodeResolver, "asset.BIN", unicodeRoot / "Asset.bin", unicodeRoot / "Asset.bin");
        const auto savedCwd = fs::current_path();
        fs::current_path(base);
        const auto relativeResolver = PathResolver::create("install");
        fs::current_path(savedCwd);
        resolves(std::get<PathResolver>(relativeResolver), "Sounds/Spell click.wav", wav, wav);
        const auto missing = resolver.resolve("Sounds/missing.wav");
        rejects(missing, ErrorCode::notFound);
        require(std::get<Error>(missing).requestedPath == "Sounds/missing.wav" &&
                std::get<Error>(missing).resolvedPath == root / "Sounds", "Diagnostic path context");
        rejects(resolver.resolve("Sounds/cafe.wav"), ErrorCode::notFound);
        rejects(resolver.resolve("missing/file.wav"), ErrorCode::notFound);
        rejects(resolver.resolve("Sounds/Spell click.wav/other"), ErrorCode::notDirectory);
        rejects(resolver.resolve("Sounds"), ErrorCode::notRegularFile);

        for (const auto& request : {"", ".", "Sounds/", "Sounds\\", "Sounds/..//Sounds/Spell click.wav",
             "Sounds/../absent", "Sounds/*", "Sounds/x?", "Sounds/x:", "Sounds/x<", "Sounds/x>",
             "Sounds/x\"", "Sounds/x|", "Sounds/x.", "Sounds/x ", "Sounds/CON", "Sounds/nul.wav",
             "Sounds/COM1.bin", "Sounds/lpt9", "Sounds/AUX .wav", "Sounds/PRN", "C:/MagicMayhem"})
            rejects(resolver.resolve(request), ErrorCode::invalidPath);
        for (const auto& request : {"/Sounds/x", "\\Sounds\\x", "//server/share/x", "\\\\?\\C:\\MagicMayhem\\x",
             "\\\\.\\device", "C:Sounds\\x", "C:", "E:/Sounds/x", "C:/MagicMayhemOther/x"})
            rejects(resolver.resolve(request), ErrorCode::unsupportedPath);
        for (const auto& request : {std::string("Sounds/a\0b", 10), std::string("Sounds/\x01"),
             std::string("Sounds/\x7f"), std::string("Sounds/\xc3\xa9")})
            rejects(resolver.resolve(request), ErrorCode::invalidPath);

        rejects(PathResolver::create({}), ErrorCode::invalidRoot);
        rejects(PathResolver::create(base / "absent"), ErrorCode::invalidRoot);
        rejects(PathResolver::create(wav), ErrorCode::invalidRoot);
        for (const auto& alias : {"relative", "C:Games", "/Games", "C:/Games/../Game", "C:/Games/.", "C:/CON"})
            rejects(PathResolver::create(root, {alias}), ErrorCode::invalidArgument);
        rejects(PathResolver::create(root, {"C:/Game", "c:/GAME"}), ErrorCode::invalidArgument);
        rejects(PathResolver::create(root, {"C:/Game", "c:/Game/Sub"}), ErrorCode::invalidArgument);
        rejects(PathResolver::create(root, {"C:/Game/Sub", "c:/Game"}), ErrorCode::invalidArgument);
        require(std::holds_alternative<PathResolver>(PathResolver::create(root, {"C:/Game", "C:/GameOther"})),
                "Component boundary alias check");
        const auto driveRoot = std::get<PathResolver>(PathResolver::create(root, {"Z:/"}));
        resolves(driveRoot, "z:/Sounds/Spell click.wav", wav, wav);

        // Case-sensitive fixture host: ambiguity wins even over an exact spelling.
        file(root / "Sounds" / "Duplicate.wav");
        file(root / "Sounds" / "duplicate.wav");
        const bool caseSensitive = !fs::equivalent(root / "Sounds/Duplicate.wav", root / "Sounds/duplicate.wav");
        if (caseSensitive) {
            rejects(resolver.resolve("Sounds/Duplicate.wav"), ErrorCode::ambiguousPath);
            fs::create_directory(root / "sounds");
            rejects(resolver.resolve("Sounds/Spell click.wav"), ErrorCode::ambiguousPath);
            fs::remove(root / "sounds");
        } else std::cout << "Case-collision fixture skipped on case-insensitive host\n";

        fs::create_directories(base / "install-other");
        file(base / "install-other" / "outside.wav");
        fs::create_symlink(wav, root / "Alias.wav");
        fs::create_directory_symlink(root / "Sounds", root / "Audio");
        resolves(resolver, "alias.WAV", root / "Alias.wav", wav);
        resolves(resolver, "audio/Spell click.wav", root / "Audio/Spell click.wav", wav);
        fs::create_symlink(base / "install-other/outside.wav", root / "Escape.wav");
        fs::create_directory_symlink(base / "install-other", root / "EscapeDir");
        rejects(resolver.resolve("Escape.wav"), ErrorCode::outsideRoot);
        rejects(resolver.resolve("EscapeDir/outside.wav"), ErrorCode::outsideRoot);
        rejects(resolver.resolve("EscapeDir/missing"), ErrorCode::outsideRoot);
        fs::create_symlink(base / "absent", root / "Broken.wav");
        rejects(resolver.resolve("Broken.wav"), ErrorCode::notFound);
        fs::create_symlink(root / "Loop.wav", root / "Loop.wav");
        rejects(resolver.resolve("Loop.wav"), ErrorCode::ioError);
        fs::create_directory_symlink(root, base / "root-alias");
        const auto rootAlias = std::get<PathResolver>(PathResolver::create(base / "root-alias"));
        resolves(rootAlias, "Sounds/Spell click.wav", wav, wav);

        // No cached listings: newly added and removed files are immediately seen.
        file(root / "New.bin");
        resolves(resolver, "new.BIN", root / "New.bin", root / "New.bin");
        fs::remove(root / "New.bin");
        rejects(resolver.resolve("New.bin"), ErrorCode::notFound);
        fs::create_directory(root / "Blocked");
        fs::permissions(root / "Blocked", fs::perms::none);
        std::error_code ec;
        fs::directory_iterator denied(root / "Blocked", ec);
        const auto deniedResult = resolver.resolve("Blocked/missing.wav");
        fs::permissions(root / "Blocked", fs::perms::owner_all);
        if (ec == std::errc::permission_denied) rejects(deniedResult, ErrorCode::permissionDenied);
        else std::cout << "Permission fixture skipped: process can enumerate mode-000 directory\n";

        std::cout << "Asset path resolution, aliases, diagnostics and containment passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Asset path test failed: " << e.what() << '\n';
        return 1;
    }
}
