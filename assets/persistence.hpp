#pragma once
#include "asset_file.hpp"
#include <array>
#include <map>

namespace mnm::assets {
using Bytes = std::vector<std::uint8_t>;
enum class PersistenceErrorCode { malformedData, unsupportedVersion, unsupportedMode, checksumMismatch, limitExceeded, assetInput };
struct PersistenceError {
    PersistenceErrorCode code;
    std::size_t offset = 0; // Byte offset in the current layer (decoded layer for saves).
    std::string detail;
    std::optional<Error> input;
};
template<class T> using PersistenceResult = std::variant<T, PersistenceError>;
struct PersistenceLimits {
    std::uint32_t inputBytes = 64*1024*1024, decodedBytes = 64*1024*1024;
    std::uint32_t lines = 65536, listWords = 65536;
};
enum class ContainerTransform { cfgBytes, noCdSave };
struct PackedContainer {
    std::uint32_t seed = 0, mode = 0, packedChecksum = 0, decodedChecksum = 0;
    Bytes decoded;
};
PersistenceResult<PackedContainer> decodePackedContainer(const Bytes&, const PersistenceLimits& = {}, ContainerTransform = ContainerTransform::cfgBytes);
PersistenceResult<PackedContainer> loadPackedContainer(AssetFile&, const PersistenceLimits& = {}, ContainerTransform = ContainerTransform::cfgBytes);

// ASCII case-insensitive section/key names; values remain owned byte strings.
struct Config {
    std::map<std::string, std::map<std::string, std::string>> sections;
    // Shipped CFGs include title lines and empty-key annotations outside INI syntax.
    std::vector<std::pair<std::size_t, std::string>> annotations;
    const std::string* find(std::string section, std::string key) const;
};
PersistenceResult<Config> decodeConfig(const Bytes&, const PersistenceLimits& = {});
PersistenceResult<Config> loadConfig(AssetFile&, bool packed = false, const PersistenceLimits& = {});
struct RealmWizardConfig { std::int32_t icon=0, flag=0, location=0, oldRegion=0; };
struct RealmConfig {
    std::string name, nextRealm;
    std::int32_t playerWizard=0, lastRegion=0;
    std::uint32_t regionCount=0;
    std::vector<RealmWizardConfig> wizards;
    // Installed configs omit some regions; absence is explicit, never invented.
    std::vector<std::optional<std::int32_t>> regionOwners;
};
PersistenceResult<RealmConfig> decodeRealmConfig(const Bytes&, const PersistenceLimits& = {});
PersistenceResult<RealmConfig> loadRealmConfig(AssetFile&, const PersistenceLimits& = {});
PersistenceResult<std::vector<std::string>> decodeRegionNames(const Bytes&, const PersistenceLimits& = {});
PersistenceResult<std::vector<std::string>> loadRegionNames(AssetFile&, const PersistenceLimits& = {});

struct RealmState {
    Bytes raw; // Exact original-build block; unnamed fields are retained.
    std::string name, nextRealm;
    std::int32_t playerWizard=0, wizardCount=0, lastRegion=0;
    std::array<std::string,80> wizardNames{};
    std::array<std::int32_t,80> icons{}, eligibility{}, locations{}, requestedLocations{}, flags{};
    std::array<std::int32_t,20> regionOwners{};
};
PersistenceResult<RealmState> decodeRealmState(const Bytes&, const PersistenceLimits& = {});
PersistenceResult<RealmState> loadRealmState(AssetFile&, const PersistenceLimits& = {});
struct SavedWizard {
    Bytes prefix; // 0x14a bytes, retained without speculative field names.
    std::vector<std::array<std::uint32_t,5>> entries; // Host stride is 24; disk stride is 20.
    Bytes records; // Ten 100-byte records.
    std::array<std::uint32_t,2> listHeader{};
    std::vector<std::uint32_t> list;
    std::optional<Bytes> extension; // Presence byte followed by 0x57c bytes.
};
struct SavedGame {
    std::uint32_t version=20, accountingValue=0;
    Bytes paths;
    std::array<SavedWizard,80> wizards;
    RealmState realm;
    Bytes controller, globals, scriptState;
    std::array<std::uint32_t,2> counters{};
    std::uint32_t worldMarker=0;
    std::optional<Bytes> worldGlobals;
    Bytes worldTail; // Opaque serialized world, NOT a restored simulation.
};
PersistenceResult<SavedGame> decodeSavedGame(const Bytes& decoded, const PersistenceLimits& = {});
PersistenceResult<SavedGame> loadSavedGame(AssetFile&, bool packed = true, const PersistenceLimits& = {});
}
