#include "sprite_player.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace mnm::sprites {
namespace {
assets::AssetStore store(const std::filesystem::path& root) {
    auto r=assets::AssetStore::create(root);
    if(auto* e=std::get_if<assets::Error>(&r))throw std::runtime_error(e->detail);
    return std::get<assets::AssetStore>(std::move(r));
}
std::unique_ptr<assets::AssetFile> file(const assets::AssetStore& s,const std::string& path) {
    auto r=s.open(path);
    if(auto* e=std::get_if<assets::Error>(&r))throw std::runtime_error(path+": "+e->detail);
    return std::get<std::unique_ptr<assets::AssetFile>>(std::move(r));
}
}
SpritePlayer::SpritePlayer(std::filesystem::path root):root_(store(root).root()) {}
std::vector<SpriteGroup> SpritePlayer::groups(const SpriteTable& table) {
    std::vector<SpriteGroup> result;
    for(const auto& f:table.sprite()->frames) {
        auto i=std::find_if(result.begin(),result.end(),[&](const auto& g){return g.name==f.name;});
        if(i==result.end())result.push_back({f.name,1});else ++i->count;
    }
    return result;
}
std::pair<std::shared_ptr<const SpriteTable>,std::string> SpritePlayer::acquire(const std::string& path) const {
    auto input=store(root_);auto tag=file(input,path);
    // An existing same-stem companion wins. Only absence allows Terrain.spr;
    // malformed/ambiguous/unreadable companions never silently fall through.
    auto normalized=path;std::replace(normalized.begin(),normalized.end(),'\\','/');
    auto candidate=std::filesystem::path(normalized);auto ext=candidate.extension().string();
    std::transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){return char(std::tolower(c));});
    if(ext!=".tag")throw std::invalid_argument("Choose a TAG sprite table");
    candidate.replace_extension(".spr");auto sprPath=candidate.generic_string();auto opened=input.open(sprPath);
    if(auto* e=std::get_if<assets::Error>(&opened)) {
        if(e->code!=assets::ErrorCode::notFound)throw std::runtime_error(e->detail);
        sprPath=(candidate.parent_path()/"Terrain.spr").generic_string();opened=input.open(sprPath);
    }
    if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(sprPath+": "+e->detail);
    auto sprite=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));
    return {SpriteTable::load(*sprite,*tag),sprPath};
}
void SpritePlayer::open(const std::string& path) {
    auto loaded=acquire(path);auto names=groups(*loaded.first);
    std::vector<TimelineEntry> entries;
    for(std::uint32_t i=0;i<names.front().count;++i)entries.push_back({names.front().name,i,100,0});
    auto next=std::make_unique<SpriteTimeline>(std::make_shared<const TimelineClip>(loaded.first,std::move(entries)),TimelineMode::loop);
    auto ownedPath=path; // All potentially allocating work precedes publication.
    tagPath_.swap(ownedPath);spritePath_.swap(loaded.second);groups_.swap(names);timeline_.swap(next);
}
const SpriteTimeline& SpritePlayer::timeline() const {
    if(!timeline_)throw std::logic_error("Open a sprite table first");
    return *timeline_;
}
void SpritePlayer::reload() {
    auto previous=timeline().checkpoint();auto loaded=acquire(tagPath_);auto names=groups(*loaded.first);
    auto next=std::make_unique<SpriteTimeline>(previous.clip,previous.mode,previous.reverse);
    next->restore(previous);next->reload(loaded.first);
    spritePath_.swap(loaded.second);groups_.swap(names);timeline_.swap(next);
}
void SpritePlayer::select(std::size_t group,TimelineMode mode,bool reverse) {
    const auto& name=groups_.at(group);auto previous=timeline().checkpoint();std::vector<TimelineEntry> entries;
    for(std::uint32_t i=0;i<name.count;++i)entries.push_back({name.name,i,100,0});
    auto next=std::make_unique<SpriteTimeline>(std::make_shared<const TimelineClip>(previous.clip->table(),std::move(entries)),mode,reverse);
    next->pause(previous.paused);timeline_.swap(next);
}
void SpritePlayer::pause(bool value) {timeline();timeline_->pause(value);}
void SpritePlayer::advance(std::uint64_t ticks) {timeline();timeline_->advance(ticks);}
void SpritePlayer::seek(std::size_t entry) {timeline();timeline_->seekEntry(entry,0);}
render::SpriteDrawPlan SpritePlayer::plan() const {return timeline().plan(previewWidth/2,previewHeight/2);}
render::Image renderPreview(render::GlBlitter& renderer,const render::SpriteDrawPlan& plan) {
    render::Image blank{previewWidth,previewHeight,std::vector<std::uint32_t>(previewWidth*previewHeight,0x2104)};
    auto surface=renderer.create(blank,render::spriteFormat);
    try {
        render::drawSpriteBatch(renderer,surface,plan,{0,0,previewWidth,previewHeight});
        auto result=renderer.read(surface);renderer.destroy(surface);return result;
    } catch(...) {renderer.destroy(surface);throw;}
}
}
