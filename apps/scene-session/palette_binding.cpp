#include "palette_binding.hpp"
#include <stdexcept>
namespace mnm::scene {
ScenePaletteBindings::ScenePaletteBindings(assets::ResourceManager& resources,PaletteBindingLimits limits):resources_(resources),limits_(limits),thread_(std::this_thread::get_id()){
 if(!limits.resources||limits.resources>4096||!limits.bytes||limits.bytes>1024ULL*1024*1024)throw std::invalid_argument("Scene palette binding bounds");
}
void ScenePaletteBindings::checkThread() const {if(thread_!=std::this_thread::get_id())throw std::runtime_error("Scene palette service owner thread");}
const ScenePaletteBindings::Binding& ScenePaletteBindings::current(const assets::ResourceId& id) const {
 checkThread();const auto& binding=bindings_.at(id);if(resources_.resident(id).revision!=binding.revision)throw std::invalid_argument("Stale scene palette resource revision");return binding;
}
ScenePaletteBindings::Binding& ScenePaletteBindings::current(const assets::ResourceId& id){return const_cast<Binding&>(static_cast<const ScenePaletteBindings&>(*this).current(id));}
void ScenePaletteBindings::bind(const assets::ResourceId& id){
 checkThread();const auto& resource=resources_.resident(id);const auto* sprite=std::get_if<assets::Sprite>(&resource.image);if(!sprite||!resource.revision)throw std::invalid_argument("Scene palette binding requires resident SPR revision");
 const auto previous=bindings_.find(id);if(previous==bindings_.end()&&bindings_.size()>=limits_.resources)throw std::length_error("Scene palette resource budget");
 Binding next{resource.revision,reconstruction::PaletteEffectChain(paletteSources(*sprite),limits_.perResource)};const auto prior=previous==bindings_.end()?0:previous->second.chain.retainedBytes();const auto other=bytes_-prior;
 if(next.chain.retainedBytes()>limits_.bytes-other)throw std::length_error("Scene palette metadata budget");
 const auto charge=next.chain.retainedBytes();bindings_.insert_or_assign(id,std::move(next));bytes_=other+charge;
}
std::optional<std::size_t> ScenePaletteBindings::append(const assets::ResourceId& id,unsigned mode,reconstruction::PaletteShadingConfig config,const reconstruction::PaletteEffectProfile& profile){
 auto& binding=current(id);if(mode>9)return {};
 // Stage every palette/source before publishing; a global-budget failure cannot
 // leave one palette or resource updated while another still uses old values.
 auto next=binding.chain;const auto ordinal=next.append(mode,config,profile);const auto other=bytes_-binding.chain.retainedBytes();
 if(next.retainedBytes()>limits_.bytes-other)throw std::length_error("Scene palette retained table budget");
 const auto charge=next.retainedBytes();binding.chain=std::move(next);bytes_=other+charge;return ordinal;
}
void ScenePaletteBindings::apply(render::SceneDraw& draw,std::size_t ordinal,std::int32_t shade) const {
 const auto& binding=current(draw.resource);const auto& resource=resources_.resident(draw.resource);const auto& sprite=std::get<assets::Sprite>(resource.image);const auto& frame=sprite.frames.at(draw.frame);
 if(!frame.paletteIndex||*frame.paletteIndex>=binding.chain.paletteCount())throw std::invalid_argument("Scene palette frame/palette identity");
 auto colours=binding.chain.colours(ordinal,*frame.paletteIndex,shade);draw.colours=std::move(colours);
}
void ScenePaletteBindings::retire(const assets::ResourceId& id){checkThread();const auto found=bindings_.find(id);if(found!=bindings_.end()){bytes_-=found->second.chain.retainedBytes();bindings_.erase(found);}}
void ScenePaletteBindings::clear(){checkThread();bindings_.clear();bytes_=0;}
}
