#include "palette_effect_chain.hpp"
#include <stdexcept>
#include <utility>
namespace mnm::reconstruction {
PaletteEffectChain::PaletteEffectChain(const std::vector<PaletteRgb>& sources,PaletteEffectChainLimits limits):limits_(limits){
 if(sources.size()>4||!limits.effects||limits.effects>4096||limits.bytes>1024ULL*1024*1024)throw std::invalid_argument("Palette effect chain bounds");
 const auto initial=std::uint64_t(sources.size())*sizeof(PaletteRgb)+std::uint64_t(limits.effects)*sizeof(Entry);
 if(initial>limits.bytes)throw std::length_error("Palette effect chain metadata budget");
 sources_=sources;entries_.reserve(limits.effects);
 bytes_=std::uint64_t(sources_.capacity())*sizeof(PaletteRgb)+std::uint64_t(entries_.capacity())*sizeof(Entry);
 if(bytes_>limits.bytes)throw std::length_error("Palette effect chain allocated metadata budget");
}
PaletteEffectChain::PaletteEffectChain(const PaletteEffectChain& other)
 :sources_(other.sources_),entries_(other.entries_),limits_(other.limits_){
 // A vector copy need not preserve capacity. Reserve ordinal metadata before
 // any future append and charge this copy's actual nested table allocations.
 entries_.reserve(limits_.effects);
 bytes_=std::uint64_t(sources_.capacity())*sizeof(PaletteRgb)+std::uint64_t(entries_.capacity())*sizeof(Entry);
 for(const auto& entry:entries_){
  bytes_+=std::uint64_t(entry.palettes.capacity())*sizeof(ShadedPalette);
  for(const auto& palette:entry.palettes)bytes_+=std::uint64_t(palette.tables.capacity())*sizeof(PaletteWords);
 }
 if(bytes_>limits_.bytes)throw std::length_error("Palette effect copied capacity budget");
}
PaletteEffectChain& PaletteEffectChain::operator=(PaletteEffectChain other){
 sources_.swap(other.sources_);entries_.swap(other.entries_);
 std::swap(limits_,other.limits_);std::swap(bytes_,other.bytes_);return *this;
}
std::optional<std::size_t> PaletteEffectChain::append(unsigned mode,PaletteShadingConfig config,const PaletteEffectProfile& profile,bool rgb555){
 if(mode>9||sources_.empty())return {};
 if(entries_.size()>=limits_.effects)throw std::length_error("Palette effect ordinal budget");
 if(config.count<2||config.count>256||(mode!=0&&mode!=8&&(config.count&(config.count-1))))throw std::invalid_argument("Palette effect chain count");
 const auto half=(config.count-1)/2;
 // Mode8 uses the normal builder's retained allocation capacity before dropping
 // its bright wing; charge that actual capacity rather than only visible tables.
 const auto tables=mode==9?half+1:config.count==3?2:2*half+1;
 const auto estimate=std::uint64_t(sources_.size())*(sizeof(ShadedPalette)+std::uint64_t(tables)*sizeof(PaletteWords));
 if(estimate>limits_.bytes-bytes_)throw std::length_error("Palette effect table budget");
 Entry next;next.mode=mode;next.entryBytes=mode>=8?4:2;next.palettes.reserve(sources_.size());auto sources=sources_;
 std::uint64_t charge=std::uint64_t(next.palettes.capacity())*sizeof(ShadedPalette);
 for(std::size_t i=0;i<sources.size();++i){
  auto result=buildPaletteEffect(sources[i],mode,config,profile,rgb555);sources[i]=result.source;
  charge+=std::uint64_t(result.palette.tables.capacity())*sizeof(PaletteWords);
  next.palettes.push_back(std::move(result.palette));
 }
 if(charge>limits_.bytes-bytes_)throw std::length_error("Palette effect allocated table budget");
 const auto ordinal=entries_.size();entries_.push_back(std::move(next));sources_.swap(sources);bytes_+=charge;return ordinal;
}
PaletteWords PaletteEffectChain::colours(std::size_t ordinal,std::size_t palette,std::int32_t shade) const {
 const auto& set=tableSet(ordinal,palette);return set.tables.at(set.tableIndex(shade));
}
}
