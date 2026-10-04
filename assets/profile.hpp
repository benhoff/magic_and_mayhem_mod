#pragma once
#include "asset_file.hpp"
#include <map>
#include <optional>
namespace mnm::assets {
inline constexpr std::int64_t ProfileInputLimit=1024*1024;
struct ProfileListing {std::uint32_t returned=0;std::vector<std::string> entries;};
// Owned ASCII snapshot; no registry mapping, encoding conversion or writes.
// Duplicate sections/keys and malformed input reject under native policy.
class ProfileSnapshot {
public:
    static ProfileSnapshot parse(const std::vector<std::uint8_t>&);
    ProfileListing section(std::string_view name,std::uint32_t capacity) const;
    std::string value(std::string_view section,std::string_view key,std::uint32_t capacity,
                      std::string_view fallback={}) const;
    // Native policy: complete unsigned decimal; missing key uses fallback.
    std::uint32_t integer(std::string_view section,std::string_view key,std::uint32_t fallback) const;
private:
    struct Entry {std::string key,raw;std::optional<std::string> value;};
    std::map<std::string,std::vector<Entry>> sections_;
};
Result<ProfileSnapshot> loadProfile(AssetFile&);
}
