#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
struct PcxImage {
    std::uint32_t width=0, height=0;
    std::uint16_t originX=0, originY=0, horizontalDpi=0, verticalDpi=0;
    std::uint16_t bytesPerLine=0, paletteInfo=0;
    std::uint64_t sourceBytes=0;
    std::array<std::array<std::uint8_t,3>,256> palette{}; // RGB, no implied alpha.
    std::vector<std::uint8_t> indices; // Top-down, tightly packed; padding removed.
};
struct PcxLimits {
    std::uint64_t inputBytes=64*1024*1024, pixels=16*1024*1024;
    std::uint32_t width=8192, height=8192;
};
enum class PcxErrorCode { invalidArgument, invalidFormat, unsupportedEncoding, malformedData, limitExceeded, assetInput };
struct PcxError {
    PcxErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using PcxResult=std::variant<PcxImage,PcxError>;
// Version 5, RLE, 8 bits, one plane, terminal 256-entry RGB palette.
// Other PCX modes return unsupportedEncoding. No transparency policy is imposed.
PcxResult decodePcx(const std::vector<std::uint8_t>&,const PcxLimits& limits={});
PcxResult loadPcx(AssetFile&,const PcxLimits& limits={});
}
