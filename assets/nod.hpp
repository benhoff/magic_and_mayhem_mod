#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
struct NodConnection {
    std::uint32_t value=0; // Nonzero slot selected by original lookup; exact meaning unresolved.
    std::int32_t target=0; // Stored local node ordinal, not a host pointer.
    std::array<std::uint8_t,17> metadata{}; // Remaining bytes of each packed 25-byte slot.
};
struct NodNode {
    std::uint32_t state=0,word4=0;
    std::array<std::int32_t,3> position{};
    std::array<NodConnection,18> connections{};
    std::array<std::uint32_t,6> tailWords{}; // File record +470..+493; meanings not inferred.
};
struct NodAsset {
    std::uint32_t version=0;
    std::uint64_t sourceBytes=0;
    std::vector<NodNode> nodes;
    std::array<std::uint32_t,2> trailerWords{}; // Preserved, not used as header/count fields.
};
struct NodLimits {
    std::uint64_t inputBytes=16*1024*1024,decodedBytes=32*1024*1024;
    std::uint32_t nodes=32768;
};
enum class NodErrorCode {invalidArgument,invalidFormat,unsupportedVersion,malformedData,limitExceeded,assetInput};
struct NodError {
    NodErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using NodResult=std::variant<NodAsset,NodError>;
// Version 1: 16-byte header, count*494 packed node records, eight-byte trailer.
// Raw input only; no relocation, section transforms, graph assembly or search.
NodResult decodeNod(const std::vector<std::uint8_t>&,const NodLimits& limits={});
NodResult loadNod(AssetFile&,const NodLimits& limits={});
}
