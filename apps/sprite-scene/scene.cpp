#include "scene.hpp"
#include <algorithm>
#include <stdexcept>
#include <limits>

namespace mnm::preview {
static reconstruction::AnimationOffset anchor(std::int64_t x,std::int64_t y){
    if(x<std::numeric_limits<std::int32_t>::min() || x>std::numeric_limits<std::int32_t>::max() ||
       y<std::numeric_limits<std::int32_t>::min() || y>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Scene anchor exceeds native coordinate bounds");
    return {std::int32_t(x),std::int32_t(y)};
}
render::Image SpriteScene::background(){
    render::Image image{512,256,std::vector<std::uint32_t>(512*256)};
    for(int y=0;y<256;++y)for(int x=0;x<512;++x)image.pixels[std::size_t(y)*512+x]=((x/16+y/16)%2)?0x2124:0x2945;
    return image;
}
SpriteScene::SpriteScene(render::GlBlitter& renderer,assets::Sprite sprite,const assets::Animation& animation,
                         const std::vector<std::uint32_t>& sequences,bool loop,ScenePlacement placement,std::vector<SpriteLayer> layers)
    :renderer_(renderer),sprite_(std::move(sprite)),animation_(animation),placement_(placement),layerAssets_(std::move(layers)),loop_(loop){
    if(sequences.empty() || sequences.size()>4)throw std::runtime_error("Scene needs 1..4 explicit ANI sequences");
    if(layerAssets_.size()>2)throw std::runtime_error("Scene supports at most two attachment layers");
    for(auto sequence:sequences){
        if(animation.starts.size()<2 || sequence>=animation.starts.size()-1)throw std::runtime_error("Scene sequence index out of range");
        const auto first=animation.starts[sequence],end=animation.starts[sequence+1];
        if(first>=end || end>animation.records.size())throw std::runtime_error("Scene sequence extent invalid");
        std::vector<assets::AnimationRecord> records(animation.records.begin()+first,animation.records.begin()+end);
        for(const auto& r:records)if(r.opcode==0 && (r.argument<0 || std::uint32_t(r.argument)>=sprite_.frames.size()))
            throw std::runtime_error("Animation sprite index outside paired SPR");
        players_.emplace_back(std::move(records));players_.back().start();
        actors_.push_back({sequence,int((actors_.size()+1)*512/(sequences.size()+1)),190,{},0,{}});
        layerPlayers_.emplace_back();layerEvents_.emplace_back();
        health_.push_back(1);attachmentModes_.push_back(1);
        for(const auto& layer:layerAssets_){
            const auto& a=layer.animation;const auto s=layer.sequence;
            if(a.starts.size()<2 || s>=a.starts.size()-1 || a.starts[s]>=a.starts[s+1] || a.starts[s+1]>a.records.size())throw std::runtime_error("Layer sequence extent invalid");
            std::vector<assets::AnimationRecord> selected(a.records.begin()+a.starts[s],a.records.begin()+a.starts[s+1]);
            for(const auto& r:selected)if(r.opcode==0 && (r.argument<0 || std::uint32_t(r.argument)>=layer.sprite.frames.size()))throw std::runtime_error("Layer sprite index outside paired SPR");
            if(layer.attachment!=reconstruction::AttachmentPoint::first && layer.attachment!=reconstruction::AttachmentPoint::second)throw std::runtime_error("Unknown layer attachment");
            if(layer.modeOne && layer.attachment!=reconstruction::AttachmentPoint::first)throw std::runtime_error("Mode-one attachment uses the first point");
            layerPlayers_.back().emplace_back(std::move(selected));layerPlayers_.back().back().start();layerEvents_.back().push_back(0);
        }
    }
    const auto image=background();background_=renderer_.create(image,render::spriteFormat);
    try{canvas_=renderer_.create(image,render::spriteFormat);}catch(...){renderer_.destroy(background_);background_=0;throw;}
}
std::vector<assets::AnimationRecord> SpriteScene::sequence(std::uint32_t index) const{
    if(animation_.starts.size()<2 || index>=animation_.starts.size()-1)throw std::runtime_error("Sequence index out of range");
    const auto a=animation_.starts[index],z=animation_.starts[index+1];
    if(a>=z || z>animation_.records.size())throw std::runtime_error("Sequence extent invalid");
    std::vector<assets::AnimationRecord> result(animation_.records.begin()+a,animation_.records.begin()+z);
    for(const auto& r:result)if(r.opcode==0 && (r.argument<0 || std::uint32_t(r.argument)>=sprite_.frames.size()))
        throw std::runtime_error("Sequence references sprite outside paired SPR");
    return result;
}
std::vector<std::uint32_t> SpriteScene::directionalGroups() const{
    std::vector<std::uint32_t> groups;
    for(std::uint32_t base=0;std::size_t(base)+8<animation_.starts.size();base+=8){
        bool compatible=true;
        try{
            const auto first=sequence(base);
            if(first.front().opcode==6)continue;
            for(unsigned facing=1;facing<8 && compatible;++facing){const auto other=sequence(base+facing);
                if(first.size()!=other.size()){compatible=false;break;}
                for(std::size_t i=0;i<first.size();++i)if(first[i].opcode!=other[i].opcode ||
                    (first[i].opcode!=0 && first[i].argument!=other[i].argument)){compatible=false;break;}
            }
        }catch(const std::runtime_error&){compatible=false;}
        if(compatible)groups.push_back(base);
    }
    return groups;
}
void SpriteScene::selectGroup(std::size_t actor,std::uint32_t base,std::uint32_t facing){
    const auto groups=directionalGroups();
    if(actor>=players_.size() || facing>=8 || std::find(groups.begin(),groups.end(),base)==groups.end())
        throw std::runtime_error("Actor/group/facing selection invalid");
    reconstruction::NoCdAnimationPlayer replacement(sequence(base+facing));replacement.start();
    players_[actor]=std::move(replacement);actors_[actor].sequence=base+facing;actors_[actor].event=0;
}
void SpriteScene::selectFacing(std::size_t actor,std::uint32_t facing){
    if(actor>=players_.size() || facing>=8)throw std::runtime_error("Actor/facing selection invalid");
    const auto base=actors_[actor].sequence-(actors_[actor].sequence%8);const auto groups=directionalGroups();
    if(std::find(groups.begin(),groups.end(),base)==groups.end())throw std::runtime_error("Actor has no verified directional group");
    players_[actor].switchSequence(sequence(base+facing));actors_[actor].sequence=base+facing;actors_[actor].event=0;
}
SpriteScene::~SpriteScene(){cache_.clear();if(canvas_)renderer_.destroy(canvas_);if(background_)renderer_.destroy(background_);}
render::UploadedSpriteFrame& SpriteScene::upload(std::uint32_t frame,std::uint32_t asset){
    const UploadKey key{asset,frame};auto found=cache_.find(key);
    if(found==cache_.end()){
        // 24 uploads consume at most 48 handles, plus background and canvas.
        if(cache_.size()==24){cache_.erase(lru_.front());lru_.erase(lru_.begin());}
        const auto& source=asset?layerAssets_.at(asset-1).sprite:sprite_;
        auto sprite=std::make_unique<render::UploadedSpriteFrame>(renderer_,source,frame);
        found=cache_.emplace(key,std::move(sprite)).first;
    }
    lru_.erase(std::remove(lru_.begin(),lru_.end(),key),lru_.end());lru_.push_back(key);
    return *found->second;
}
void SpriteScene::advance(){
    for(std::size_t i=0;i<players_.size();++i){
        if(loop_ && !players_[i].state().active){players_[i].start();actors_[i].event=0;}
        else actors_[i].event=players_[i].tick();
        for(std::size_t l=0;l<layerPlayers_[i].size();++l){auto& player=layerPlayers_[i][l];
            if(layerAssets_[l].modeOne && !reconstruction::modeOneTicks(attachmentModes_[i]))layerEvents_[i][l]=0;
            else if(loop_ && !layerAssets_[l].modeOne && !player.state().active){player.start();layerEvents_[i][l]=0;}else layerEvents_[i][l]=player.tick();}
    }
}
QImage SpriteScene::present(){
    renderer_.copy(background_,canvas_,{0,0,512,256},0,0);
    const auto states=actors();const auto attached=layers();
    for(std::size_t i=0;i<states.size();++i){const auto& actor=states[i];
        if(actor.sprite && actor.drawAnchor)upload(*actor.sprite).draw(canvas_,actor.drawAnchor->x,actor.drawAnchor->y);
        // Explicit preview ordering, not the original world sort/occlusion queue.
        for(const auto& layer:attached)if(layer.actor==i && layer.sprite && layer.drawAnchor)
            upload(*layer.sprite,std::uint32_t(layer.layer+1)).draw(canvas_,layer.drawAnchor->x,layer.drawAnchor->y);
    }
    return renderer_.present(canvas_);
}
render::Image SpriteScene::read(){return renderer_.read(canvas_);}
std::vector<ActorState> SpriteScene::actors() const{
    auto result=actors_;for(std::size_t i=0;i<result.size();++i){result[i].sprite=players_[i].sprite();const auto record=players_[i].displayedRecord();
        if(record){const auto offset=reconstruction::spriteOffset(*record,placement_.tileSizeXY,placement_.view);
            result[i].drawAnchor=anchor(std::int64_t(result[i].anchorX)+offset.x,std::int64_t(result[i].anchorY)+offset.y);}}
    return result;
}
std::vector<LayerState> SpriteScene::layers() const{
    std::vector<LayerState> result;
    for(std::size_t i=0;i<players_.size();++i)for(std::size_t l=0;l<layerAssets_.size();++l){
        LayerState state{i,l,layerAssets_[l].sequence,{},{},layerEvents_[i][l]};
        const auto parent=players_[i].displayedRecord(),child=layerPlayers_[i][l].displayedRecord();
        if(parent && child && (!layerAssets_[l].modeOne || reconstruction::modeOneVisible(health_[i],attachmentModes_[i]))){const auto point=reconstruction::attachmentOffset(*parent,layerAssets_[l].attachment,placement_.tileSizeXY,placement_.view);
            const auto local=reconstruction::spriteOffset(*child);state.sprite=layerPlayers_[i][l].sprite();
            state.drawAnchor=anchor(std::int64_t(actors_[i].anchorX)+point.x+local.x,std::int64_t(actors_[i].anchorY)+point.y+local.y);}
        result.push_back(state);
    }
    return result;
}
void SpriteScene::setCreatureHealth(std::size_t actor,std::int32_t health){health_.at(actor)=health;}
void SpriteScene::setModeOneAttachment(std::size_t actor,bool enabled){
    auto& mode=attachmentModes_.at(actor);const unsigned next=enabled?1:0;if(mode==next)return;
    for(std::size_t l=0;l<layerAssets_.size();++l)if(layerAssets_[l].modeOne){
        if(enabled)layerPlayers_[actor][l].start();else layerPlayers_[actor][l].stop();layerEvents_[actor][l]=0;}
    mode=next;
}
}
