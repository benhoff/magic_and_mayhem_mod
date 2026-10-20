#pragma once
#include "palette_effects.hpp"
namespace mnm::reconstruction {
struct PaletteEffectChainLimits {
 std::size_t effects=64;
 std::uint64_t bytes=64ULL*1024*1024;
};
// Owned RGB state and immutable completed effect entries for one sprite resource.
// Value tables can be copied directly to native renderer colour-table inputs.
// This is a thread-owned service; no pointers into original chains are retained.
class PaletteEffectChain {
 struct Entry {unsigned mode=0,entryBytes=2;std::vector<ShadedPalette> palettes;};
 std::vector<PaletteRgb> sources_;
 std::vector<Entry> entries_;
 PaletteEffectChainLimits limits_;
 std::uint64_t bytes_=0;
public:
 explicit PaletteEffectChain(const std::vector<PaletteRgb>&,PaletteEffectChainLimits={});
 // Copies preserve reserved ordinal storage and recompute actual table capacity.
 PaletteEffectChain(const PaletteEffectChain&);
 PaletteEffectChain(PaletteEffectChain&&) noexcept = default;
 PaletteEffectChain& operator=(PaletteEffectChain);
 // Unknown mode and zero palette count are successful no-ops, as in 0057d690.
 // Other failures throw before changing any existing chain or RGB source.
 std::optional<std::size_t> append(unsigned mode,PaletteShadingConfig={},
     const PaletteEffectProfile& = {},bool rgb555=false);
 std::size_t ordinals() const {return entries_.size();}
 std::size_t paletteCount() const {return sources_.size();}
 std::uint64_t retainedBytes() const {return bytes_;}
 unsigned mode(std::size_t ordinal) const {return entries_.at(ordinal).mode;}
 unsigned originalEntryBytes(std::size_t ordinal) const {return entries_.at(ordinal).entryBytes;}
 PaletteRgb source(std::size_t palette) const {return sources_.at(palette);}
 // A borrowed immutable set lives until this chain is destroyed; callers must
 // not retain its reference across replacement of the owning resource/session.
 const ShadedPalette& tableSet(std::size_t ordinal,std::size_t palette) const {return entries_.at(ordinal).palettes.at(palette);}
 PaletteWords colours(std::size_t ordinal,std::size_t palette,std::int32_t shade) const;
};
}
