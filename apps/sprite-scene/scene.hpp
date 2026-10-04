#pragma once
#include "sprite.hpp"
#include "no_cd.hpp"
#include <map>

namespace mnm::preview {
struct ActorState {std::uint32_t sequence=0;int anchorX=0,anchorY=0;std::optional<std::uint32_t> sprite;std::int32_t event=0;};
// Application orchestration only. Renderer and native asset services do not
// depend on this build-specific player, preview policies or application widgets.
class SpriteScene final {
public:
    SpriteScene(render::GlBlitter& renderer,assets::Sprite sprite,const assets::Animation& animation,
                const std::vector<std::uint32_t>& sequences,bool loop);
    ~SpriteScene();
    SpriteScene(const SpriteScene&)=delete;
    SpriteScene& operator=(const SpriteScene&)=delete;
    void advance();
    // Explicit preview selection: group changes restart; facing changes retain
    // phase. Groups have validated matching opcode/control record positions.
    std::vector<std::uint32_t> directionalGroups() const;
    void selectGroup(std::size_t actor,std::uint32_t base,std::uint32_t facing);
    void selectFacing(std::size_t actor,std::uint32_t facing);
    QImage present();
    render::Image read();
    std::vector<ActorState> actors() const;
    static render::Image background();
    const assets::Sprite& sprite() const{return sprite_;}
private:
    render::UploadedSpriteFrame& upload(std::uint32_t frame);
    render::GlBlitter& renderer_;
    assets::Sprite sprite_;
    assets::Animation animation_;
    std::vector<assets::AnimationRecord> sequence(std::uint32_t index) const;
    std::vector<reconstruction::NoCdAnimationPlayer> players_;
    std::vector<ActorState> actors_;
    std::map<std::uint32_t,std::unique_ptr<render::UploadedSpriteFrame>> cache_;
    std::vector<std::uint32_t> lru_;
    render::SurfaceId background_=0,canvas_=0;
    bool loop_=false;
};
}
