#pragma once
#include "profile.hpp"
#include <cstdint>
namespace mnm::audio {
// Session-owned read-only audio configuration. Original manager rules remain
// outside this service; duplicate/malformed and integer policies are native.
class ProfileSource {
public:
    explicit ProfileSource(mnm::assets::AssetStore);
    mnm::assets::ProfileListing section(std::string_view name,std::uint32_t capacity) const{return profile_.section(name,capacity);}
    std::string value(std::string_view section,std::string_view key,std::uint32_t capacity,std::string_view fallback={}) const{return profile_.value(section,key,capacity,fallback);}
    std::uint32_t integer(std::string_view section,std::string_view key,std::uint32_t fallback) const{return profile_.integer(section,key,fallback);}
private:
    static mnm::assets::ProfileSnapshot load(const mnm::assets::AssetStore&);
    mnm::assets::AssetStore assets_;
    mnm::assets::ProfileSnapshot profile_;
};
}
