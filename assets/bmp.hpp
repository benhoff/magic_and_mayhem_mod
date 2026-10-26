#pragma once
#include "asset_file.hpp"

namespace mnm::assets {
struct BmpImage {
    std::uint32_t width=0, height=0;
    bool sourceTopDown=false;
    std::int32_t horizontalPixelsPerMeter=0, verticalPixelsPerMeter=0;
    std::uint32_t pixelOffset=0, rowStride=0, declaredImageBytes=0;
    std::uint32_t colorsUsed=0, colorsImportant=0;
    std::uint64_t sourceBytes=0;
    std::vector<std::uint8_t> rgb; // Owned, top-down, tightly packed RGB; no alpha.
};
struct BmpLimits {
    std::uint64_t inputBytes=64*1024*1024, pixels=16*1024*1024, decodedBytes=48*1024*1024;
    std::uint32_t width=8192, height=8192;
};
enum class BmpErrorCode { invalidArgument, invalidFormat, unsupportedEncoding, malformedData, limitExceeded, assetInput };
struct BmpError {
    BmpErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using BmpResult=std::variant<BmpImage,BmpError>;
// BM with 12-byte core or 40-byte information headers. Owned RGB output.
// Indexed 1/4/8, RGB555/BGR/BGRX, 16/32-bit RGB masks, bottom-up RLE4/8.
// No alpha interpretation; unsupported DIB headers/compressions are errors.
BmpResult decodeBmp(const std::vector<std::uint8_t>&,const BmpLimits& limits={});
BmpResult loadBmp(AssetFile&,const BmpLimits& limits={});
}
