#include "sprite_session.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::sprites {
SpriteTable::SpriteTable(assets::Sprite sprite, const assets::TagAsset& tag) {
    if(sprite.frames.empty() || sprite.frames.size()>4096 || tag.entries.size()!=sprite.frames.size())
        throw std::invalid_argument("SPR/TAG companion count outside native domain");
    std::uint64_t pixels=0;
    for(std::size_t i=0; i<sprite.frames.size(); ++i) {
        const auto& f=sprite.frames[i];
        if(f.width>2048 || f.height>2048) throw std::length_error("Sprite table frame dimensions");
        pixels+=std::uint64_t(f.width)*f.height;
        if(pixels>16*1024*1024) throw std::length_error("Sprite table pixel budget");
        auto& group=names_[f.name];
        if(tag.entries[i].nameBytes!=f.name || tag.entries[i].occurrence!=group.size())
            throw std::invalid_argument("SPR/TAG exact name/occurrence differs");
        render::prepareSpriteFrame(sprite,i); // Complete renderer-domain admission.
        group.push_back(i);
    }
    sprite_=std::make_shared<const assets::Sprite>(std::move(sprite));
}
std::shared_ptr<const SpriteTable> SpriteTable::load(assets::AssetFile& spr, assets::AssetFile& tag) {
    auto s=assets::loadSprite(spr);auto t=assets::loadTag(tag);
    if(auto* e=std::get_if<assets::SpriteError>(&s)) throw std::runtime_error(e->detail);
    if(auto* e=std::get_if<assets::TagError>(&t)) throw std::runtime_error(e->detail);
    return std::make_shared<const SpriteTable>(std::get<assets::Sprite>(std::move(s)),std::get<assets::TagAsset>(std::move(t)));
}
std::size_t SpriteTable::find(const Name& name, std::uint32_t occurrence) const {
    return names_.at(name).at(occurrence);
}
std::vector<std::size_t> SpriteTable::range(const Name& name, std::uint32_t first, std::uint32_t count) const {
    const auto& frames=names_.at(name);
    if(!count || first>frames.size() || count>frames.size()-first)
        throw std::out_of_range("Named sprite clip outside occurrence group");
    return {frames.begin()+first,frames.begin()+first+count};
}
void SpriteSession::validate(const Checkpoint& s) {
    if(!s.table || !s.frameTicks || s.frameTicks>1000000000 ||
       (s.mode!=Playback::loop && s.mode!=Playback::once))
        throw std::invalid_argument("Invalid sprite playback checkpoint");
    s.table->range(s.name,s.first,s.count);
    const auto duration=s.frameTicks*s.count; // <= 1e9 * 4096.
    if(s.phase>duration || (s.mode==Playback::loop && s.phase==duration))
        throw std::invalid_argument("Sprite phase outside clip");
}
SpriteSession::SpriteSession(std::shared_ptr<const SpriteTable> table, Name name, std::uint32_t first,
                             std::uint32_t count, std::uint64_t rate, Playback mode)
    :state_{std::move(table),name,first,count,mode,rate,0} { validate(state_); }
std::size_t SpriteSession::frame() const {
    const auto ordinal=std::min<std::uint64_t>(state_.phase/state_.frameTicks,state_.count-1);
    return state_.table->find(state_.name,state_.first+std::uint32_t(ordinal));
}
bool SpriteSession::finished() const {
    return state_.mode==Playback::once && state_.phase==state_.frameTicks*state_.count;
}
void SpriteSession::advance(std::uint64_t ticks) {
    const auto duration=state_.frameTicks*state_.count;
    if(state_.mode==Playback::loop) state_.phase=(state_.phase+ticks%duration)%duration;
    else state_.phase+=std::min(ticks,duration-state_.phase);
}
void SpriteSession::restore(Checkpoint state) { validate(state);state_=std::move(state); }
void SpriteSession::reload(std::shared_ptr<const SpriteTable> table) {
    auto candidate=state_;candidate.table=std::move(table);candidate.phase=0;restore(std::move(candidate));
}
render::SpriteDrawPlan SpriteSession::plan(std::int32_t x,std::int32_t y,std::optional<render::SpriteColourTable> colours) const {
    if(colours && state_.table->sprite()->storage!=assets::SpriteStorage::indexed8)
        throw std::invalid_argument("Direct sprites do not accept palette overrides");
    return {state_.table->sprite(),{{frame(),x,y,std::move(colours)}}};
}
}
