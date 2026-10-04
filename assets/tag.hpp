#pragma once
#include "asset_file.hpp"
#include <array>

namespace mnm::assets {
struct TagEntry {
    std::array<std::uint8_t,8> nameBytes{}; // Exact bytes, including any NUL/padding.
    std::uint32_t occurrence=0; // Installed files count previous frames with the same name.
};
struct TagAsset {
    std::uint64_t sourceBytes=0;
    std::vector<TagEntry> entries; // File order equals companion SPR frame order in the tested corpus.
};
struct TagLimits {
    std::uint64_t inputBytes=4*1024*1024, decodedBytes=4*1024*1024;
    std::uint32_t records=262144;
};
enum class TagErrorCode { invalidArgument, malformedData, limitExceeded, assetInput };
struct TagError {
    TagErrorCode code;
    std::size_t offset;
    std::string detail;
    std::optional<Error> input;
};
using TagResult=std::variant<TagAsset,TagError>;
// Byte prefix before the first NUL, bounded to eight bytes; no encoding imposed.
std::string tagEntryName(const TagEntry&);
// Headerless 12-byte records: eight raw name bytes and one little-endian DWORD.
// No sequence validation, SPR dependency or resolver policy is imposed here.
TagResult decodeTag(const std::vector<std::uint8_t>&,const TagLimits& limits={});
TagResult loadTag(AssetFile&,const TagLimits& limits={});
}
