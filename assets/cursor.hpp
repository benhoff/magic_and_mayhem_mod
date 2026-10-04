#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
struct CursorImage {
    std::uint16_t width=0, height=0, hotspotX=0, hotspotY=0, bitDepth=0;
    std::uint8_t directoryColorCount=0;
    std::uint32_t sourceOffset=0, encodedSize=0;
    // RGB + original reserved byte, not alpha. No source pointers survive.
    std::vector<std::array<std::uint8_t,4>> palette;
    // Top-down, width*height pixels. Retain both planes: AND=1 does not
    // imply transparency when the palette-resolved XOR colour is nonzero.
    std::vector<std::uint8_t> xorIndices;
    std::vector<std::uint8_t> andMask; // One unpacked 0/1 bit per pixel.
};
struct CursorAsset {
    std::uint64_t sourceBytes=0;
    std::vector<CursorImage> images; // Directory order, with per-image hotspots.
};
struct CursorLimits {
    std::uint64_t inputBytes=4*1024*1024, pixels=1024*1024, decodedBytes=4*1024*1024;
    std::uint32_t images=64, width=256, height=256;
};
enum class CursorErrorCode { invalidArgument, invalidFormat, unsupportedEncoding, malformedData, limitExceeded, assetInput };
struct CursorError {
    CursorErrorCode code;
    std::size_t offset=0;
    std::optional<std::uint32_t> image;
    std::string detail;
    std::optional<Error> input;
};
using CursorResult=std::variant<CursorAsset,CursorError>;
// Indexed 1/8-bpp, uncompressed, bottom-up BITMAPINFOHEADER cursor images.
// PNG, direct-colour/alpha, compression and other DIB headers are explicit errors.
CursorResult decodeCursor(const std::vector<std::uint8_t>& bytes,const CursorLimits& limits={});
CursorResult loadCursor(AssetFile&,const CursorLimits& limits={});
}
