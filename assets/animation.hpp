#pragma once
#include "asset_file.hpp"
#include <array>
#include <cstdint>
#include <optional>

namespace mnm::assets {
struct AnimationRecord {
    std::uint32_t opcode=0;
    std::int32_t argument=0;
    // Raw +8..+43 words, including eight name bytes and coordinate-like fields.
    // v3 stores through +24; v4 through +32; absent trailing words are zero.
    // Their remaining placement/attachment semantics are not interpreted here.
    std::array<std::uint32_t,9> metadata{};
};
struct Animation {
    // Source version; records always use the normalized 44-byte representation.
    std::uint32_t version=5,opaqueHeader=0;
    std::array<std::uint8_t,20> spriteName{};
    // Record indices; final entry is a terminal extent, not another sequence.
    std::vector<std::uint32_t> starts;
    std::vector<AnimationRecord> records;
};
struct AnimationLimits {std::uint64_t inputBytes=8*1024*1024;std::uint32_t records=65536,sequences=4096;};
enum class AnimationErrorCode {invalidArgument,invalidFormat,unsupportedVersion,malformedData,limitExceeded,assetInput};
struct AnimationError {AnimationErrorCode code;std::size_t offset=0;std::string detail;std::optional<Error> input;};
using AnimationResult=std::variant<Animation,AnimationError>;
AnimationResult decodeAnimation(const std::vector<std::uint8_t>& bytes,const AnimationLimits& limits={});
AnimationResult loadAnimation(AssetFile& file,const AnimationLimits& limits={});
}
