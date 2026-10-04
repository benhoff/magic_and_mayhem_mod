#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
struct EvtArea {
    std::array<std::int32_t,3> first{}, second{}; // Section-local x,y,z endpoints, unnormalized.
    std::array<std::uint8_t,48> nameBytes{}; // Includes all bytes after the first NUL.
};
struct EvtAsset {
    std::uint32_t headerSizeWord=0, version=0;
    std::uint64_t sourceBytes=0;
    std::vector<EvtArea> areas; // File order, owned values.
};
struct EvtLimits {
    std::uint64_t inputBytes=4*1024*1024, decodedBytes=4*1024*1024;
    std::uint32_t records=65536;
};
enum class EvtErrorCode { invalidArgument, invalidFormat, unsupportedVersion, malformedData, limitExceeded, assetInput };
struct EvtError {
    EvtErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using EvtResult=std::variant<EvtAsset,EvtError>;
// Byte prefix before the first NUL, bounded to 48 bytes; no text encoding imposed.
std::string evtAreaName(const EvtArea&);
// Version 1, 16-byte header and exactly count*72 record bytes.
// The header size word is metadata, not the actual file extent.
EvtResult decodeEvt(const std::vector<std::uint8_t>&,const EvtLimits& limits={});
EvtResult loadEvt(AssetFile&,const EvtLimits& limits={});
}
