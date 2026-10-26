#pragma once
#include "../sprite-timeline/sprite_timeline.hpp"

namespace mnm::sprites {
struct SpriteGroup { Name name{}; std::uint32_t count=0; };
// Read-only installed-asset application service. No widgets, original object
// addresses, ANI interpretation or automatic dependency on TAG in the game.
class SpritePlayer final {
    std::filesystem::path root_;
    std::string tagPath_, spritePath_;
    std::vector<SpriteGroup> groups_;
    std::unique_ptr<SpriteTimeline> timeline_;
    static std::vector<SpriteGroup> groups(const SpriteTable&);
    std::pair<std::shared_ptr<const SpriteTable>,std::string> acquire(const std::string&) const;
public:
    explicit SpritePlayer(std::filesystem::path root);
    void open(const std::string& tagPath);
    // Retains the complete selected clip and temporal state, including pause.
    // All I/O/companion/clip admission happens before publication.
    void reload();
    void select(std::size_t group,TimelineMode mode=TimelineMode::loop,bool reverse=false);
    const auto& groups() const {return groups_;}
    const auto& tagPath() const {return tagPath_;}
    const auto& spritePath() const {return spritePath_;}
    const SpriteTimeline& timeline() const;
    void pause(bool value);
    void advance(std::uint64_t milliseconds);
    void seek(std::size_t scheduleEntry);
    render::SpriteDrawPlan plan() const;
};
// Synchronous offscreen OpenGL -> owned RGB565 image. The Qt view chooses how
// to present it; GPU surfaces never cross the application boundary.
render::Image renderPreview(render::GlBlitter&,const render::SpriteDrawPlan&);
inline constexpr int previewWidth=320,previewHeight=240;
}
