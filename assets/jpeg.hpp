#pragma once
#include "asset_file.hpp"

namespace mnm::assets {
struct JpegImage {
    std::uint32_t width=0, height=0;
    std::uint64_t sourceBytes=0;
    std::vector<std::uint8_t> rgb; // Owned, top-down, tightly packed RGB; no alpha.
};
struct JpegLimits {
    std::uint64_t inputBytes=64*1024*1024, pixels=16*1024*1024;
    std::uint64_t decodedBytes=48*1024*1024, codecImageBytes=64*1024*1024;
    std::uint32_t width=8192, height=8192;
};
enum class JpegErrorCode { invalidArgument, invalidFormat, codecUnavailable, malformedData, limitExceeded, assetInput };
struct JpegError {
    JpegErrorCode code;
    std::string detail;
    std::optional<Error> input;
};
using JpegResult=std::variant<JpegImage,JpegError>;
// Qt JPEG codec backend, hidden behind standard-library owned storage.
// Encoded orientation, no EXIF rotation/scaling or ICC color conversion.
JpegResult decodeJpeg(const std::vector<std::uint8_t>&,const JpegLimits& limits={});
JpegResult loadJpeg(AssetFile&,const JpegLimits& limits={});
}
