#include "scene.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::preview {
render::Image SpriteScene::background(){
    render::Image image{512,256,std::vector<std::uint32_t>(512*256)};
    for(int y=0;y<256;++y)for(int x=0;x<512;++x)image.pixels[std::size_t(y)*512+x]=((x/16+y/16)%2)?0x2124:0x2945;
    return image;
}
SpriteScene::SpriteScene(render::GlBlitter& renderer,assets::Sprite sprite,const assets::Animation& animation,
                         const std::vector<std::uint32_t>& sequences,bool loop)
    :renderer_(renderer),sprite_(std::move(sprite)),animation_(animation),loop_(loop){
    if(sequences.empty() || sequences.size()>4)throw std::runtime_error("Scene needs 1..4 explicit ANI sequences");
    for(auto sequence:sequences){
        if(animation.starts.size()<2 || sequence>=animation.starts.size()-1)throw std::runtime_error("Scene sequence index out of range");
        const auto first=animation.starts[sequence],end=animation.starts[sequence+1];
        if(first>=end || end>animation.records.size())throw std::runtime_error("Scene sequence extent invalid");
        std::vector<assets::AnimationRecord> records(animation.records.begin()+first,animation.records.begin()+end);
        for(const auto& r:records)if(r.opcode==0 && (r.argument<0 || std::uint32_t(r.argument)>=sprite_.frames.size()))
            throw std::runtime_error("Animation sprite index outside paired SPR");
        players_.emplace_back(std::move(records));players_.back().start();
        actors_.push_back({sequence,int((actors_.size()+1)*512/(sequences.size()+1)),190,{},0});
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
render::UploadedSpriteFrame& SpriteScene::upload(std::uint32_t frame){
    auto found=cache_.find(frame);
    if(found==cache_.end()){
        // 24 uploads consume at most 48 handles, plus background and canvas.
        if(cache_.size()==24){cache_.erase(lru_.front());lru_.erase(lru_.begin());}
        auto sprite=std::make_unique<render::UploadedSpriteFrame>(renderer_,sprite_,frame);
        found=cache_.emplace(frame,std::move(sprite)).first;
    }
    lru_.erase(std::remove(lru_.begin(),lru_.end(),frame),lru_.end());lru_.push_back(frame);
    return *found->second;
}
void SpriteScene::advance(){
    for(std::size_t i=0;i<players_.size();++i){
        if(loop_ && !players_[i].state().active){players_[i].start();actors_[i].event=0;}
        else actors_[i].event=players_[i].tick();
    }
}
QImage SpriteScene::present(){
    renderer_.copy(background_,canvas_,{0,0,512,256},0,0);
    for(std::size_t i=0;i<players_.size();++i){const auto frame=players_[i].sprite();
        if(frame)upload(*frame).draw(canvas_,actors_[i].anchorX,actors_[i].anchorY);}
    return renderer_.present(canvas_);
}
render::Image SpriteScene::read(){return renderer_.read(canvas_);}
std::vector<ActorState> SpriteScene::actors() const{
    auto result=actors_;for(std::size_t i=0;i<result.size();++i)result[i].sprite=players_[i].sprite();return result;
}
}
