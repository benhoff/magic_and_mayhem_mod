#pragma once
#include "scene_renderer.hpp"
#include "no_cd.hpp"
#include "placement.hpp"
#include "attachment.hpp"
#include "sprite_queue.hpp"
#include "sprite_visibility.hpp"


namespace mnm::preview {
struct ActorState {std::uint32_t sequence=0;int anchorX=0,anchorY=0;std::optional<std::uint32_t> sprite;std::int32_t event=0;std::optional<reconstruction::AnimationOffset> drawAnchor;};
struct ScenePlacement {std::uint32_t tileSizeXY=1,view=0;};
struct SceneQueueInput {reconstruction::SpriteDepth position;std::array<std::int32_t,3> priorityBias{{6,8,9}};};
struct SceneDraw {std::size_t actor=0;std::uint32_t asset=0,frame=0;reconstruction::AnimationOffset anchor;std::int32_t key=0,kind=0;};
struct SpriteLayer {assets::ResourceId resource;std::uint32_t sequence=0;reconstruction::AttachmentPoint attachment=reconstruction::AttachmentPoint::first;bool modeOne=false;};
struct LayerState {std::size_t actor=0,layer=0;std::uint32_t sequence=0;std::optional<std::uint32_t> sprite;std::optional<reconstruction::AnimationOffset> drawAnchor;std::int32_t event=0;};
// Application orchestration only. Renderer and native asset services do not
// depend on this build-specific player, preview policies or application widgets.
class SpriteScene final {
public:
    SpriteScene(render::GlBlitter& renderer,assets::ResourceManager& resources,assets::ResourceId body,
                const std::vector<std::uint32_t>& sequences,bool loop,ScenePlacement placement={},std::vector<SpriteLayer> layers={});
    ~SpriteScene();
    SpriteScene(const SpriteScene&)=delete;
    SpriteScene& operator=(const SpriteScene&)=delete;
    void advance();
    // Explicit preview selection: group changes restart; facing changes retain
    // phase. Groups have validated matching opcode/control record positions.
    std::vector<std::uint32_t> directionalGroups() const;
    void selectGroup(std::size_t actor,std::uint32_t base,std::uint32_t facing);
    void selectFacing(std::size_t actor,std::uint32_t facing);
    void setQueueInput(std::size_t actor,SceneQueueInput input);
    void setAnchor(std::size_t actor,int x,int y);
    void setVisibility(bool enabled,bool expanded=false){visibility_=enabled;expandedVisibility_=expanded;}
    std::vector<SceneDraw> drawQueue() const;
    QImage present();
    render::Image read();
    std::vector<ActorState> actors() const;
    std::vector<LayerState> layers() const;
    void setCreatureHealth(std::size_t actor,std::int32_t health);
    void setModeOneAttachment(std::size_t actor,bool enabled);
    static render::Image background();
    const assets::Sprite& sprite() const{return sprite_;}
private:
    assets::ResourceManager& resources_;
    assets::ResourceId body_;
    const assets::Sprite& sprite_;
    const assets::Animation& animation_;
    const assets::Sprite& layerSprite(std::size_t) const;
    std::vector<assets::AnimationRecord> sequence(std::uint32_t index) const;
    std::vector<reconstruction::NoCdAnimationPlayer> players_;
    std::vector<ActorState> actors_;
    ScenePlacement placement_;
    std::vector<SceneQueueInput> queueInputs_;
    std::vector<SpriteLayer> layerAssets_;
    std::vector<std::vector<reconstruction::NoCdAnimationPlayer>> layerPlayers_;
    std::vector<std::vector<std::int32_t>> layerEvents_;
    std::vector<std::int32_t> health_;
    std::vector<std::uint32_t> attachmentModes_;
    std::unique_ptr<render::SceneRenderer> drawing_;
    bool loop_=false,visibility_=false,expandedVisibility_=false;
};
}
