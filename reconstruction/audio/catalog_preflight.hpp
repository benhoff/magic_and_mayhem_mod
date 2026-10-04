#pragma once
#include "native_manager_backend.hpp"
namespace mnm::reconstruction::audio {
struct SourceProbe {
    std::int32_t id=0;
    std::string path,diagnostic;
    bool playable=false;
    std::uint32_t pcmBytes=0,durationMs=0;
};
struct GroupProbe {
    std::int32_t id=0;
    std::vector<std::int32_t> members,resolved;
    bool playable=false;
    std::string diagnostic;
};
struct CatalogPreflight {
    NativeSourcePathPolicy policy=NativeSourcePathPolicy::literal;
    std::vector<SourceProbe> sources;
    std::vector<GroupProbe> groups;
    std::uint32_t simultaneousLimit=0;
    // Fatal profile/catalog errors; individual asset failures remain reportable.
    std::string diagnostic;
    bool inspected=false;
    bool valid() const{return inspected && diagnostic.empty();}
    bool playable(std::int32_t requested) const;
};
// Read-only native startup check, no primary buffers, source pool or audio sink.
// Groups are logical requests, not fictitious WAV filenames. Probe every choice
// with the recovered one-step selection and unknown-member fallback to ID 410.
CatalogPreflight preflightCatalog(mnm::assets::AssetStore,NativeSourcePathPolicy);
}
