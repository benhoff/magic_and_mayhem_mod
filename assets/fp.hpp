#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
struct FpPoint {std::int32_t x=0,y=0;};
struct FpAsset {
    std::uint32_t version=0,pathCount=0;
    std::uint64_t sourceBytes=0;
    FpPoint headerPoint; // Header offsets 8/12; exact caller meaning not established.
    std::array<std::uint32_t,8> pointCounts{},pointOffsets{}; // Offsets in points, not bytes.
    std::array<FpPoint,4> flagPositions{};
    std::vector<FpPoint> points; // Owned flat payload; stored path ranges preserved.
};
struct FpLimits {
    std::uint64_t inputBytes=4*1024*1024,decodedBytes=4*1024*1024;
    std::uint32_t points=65536;
};
enum class FpErrorCode { invalidArgument,invalidFormat,unsupportedVersion,malformedData,limitExceeded,assetInput };
struct FpError {
    FpErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using FpResult=std::variant<FpAsset,FpError>;
// Version 2: 116-byte header, followed by eight-byte x/y point records.
// Active path ranges must fit the payload. Inactive slots are retained verbatim.
FpResult decodeFp(const std::vector<std::uint8_t>&,const FpLimits& limits={});
FpResult loadFp(AssetFile&,const FpLimits& limits={});
}
