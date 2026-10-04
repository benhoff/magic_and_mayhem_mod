#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
enum class MpsKind : std::int32_t {
    undefined=0, friendlyWizard=1, enemyWizard=2, multiplayerWizard=3, creature=4, artifact=5
};
struct MpsPlacement {
    std::array<std::int32_t,3> position{}; // Section-local x,y,z; no world transform.
    MpsKind kind=MpsKind::undefined; // Unknown numeric values retained.
    std::array<std::int32_t,6> parameters{}; // Kind-specific semantics not fully recovered.
};
struct MpsAsset {
    std::uint32_t headerSizeWord=0, version=0;
    std::uint64_t sourceBytes=0;
    std::vector<MpsPlacement> placements; // File order, owned values.
};
struct MpsLimits {
    std::uint64_t inputBytes=4*1024*1024, decodedBytes=4*1024*1024;
    std::uint32_t records=65536;
};
enum class MpsErrorCode { invalidArgument, invalidFormat, unsupportedVersion, malformedData, limitExceeded, assetInput };
struct MpsError {
    MpsErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using MpsResult=std::variant<MpsAsset,MpsError>;
const char* mpsKindName(MpsKind);
// Version 1, 16-byte header and exactly count*40 record bytes.
// The header size word is metadata, not the actual file extent.
MpsResult decodeMps(const std::vector<std::uint8_t>&,const MpsLimits& limits={});
MpsResult loadMps(AssetFile&,const MpsLimits& limits={});
}
