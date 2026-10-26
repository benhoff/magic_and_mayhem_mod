#pragma once
#include "../../assets/tag.hpp"
#include "../../renderer/sprites/sprite_batch.hpp"
#include <map>

namespace mnm::sprites {
using Name = std::array<std::uint8_t, 8>;
// Native companion policy, independent of original TAG lookup/ANI timing.
class SpriteTable final {
    std::shared_ptr<const assets::Sprite> sprite_;
    std::map<Name, std::vector<std::size_t>> names_;
public:
    SpriteTable(assets::Sprite, const assets::TagAsset&);
    static std::shared_ptr<const SpriteTable> load(assets::AssetFile& spr, assets::AssetFile& tag);
    const auto& sprite() const { return sprite_; }
    std::size_t find(const Name&, std::uint32_t occurrence) const;
    std::vector<std::size_t> range(const Name&, std::uint32_t first, std::uint32_t count) const;
};
enum class Playback { loop, once };
struct Checkpoint {
    std::shared_ptr<const SpriteTable> table;
    Name name{};
    std::uint32_t first=0, count=0;
    Playback mode=Playback::loop;
    std::uint64_t frameTicks=1, phase=0;
};
// Thread-confined session. Integer ticks are supplied by the host, not sampled
// from a clock. Immutable plans/checkpoints may outlive reload and the session.
class SpriteSession final {
    Checkpoint state_;
    static void validate(const Checkpoint&);
public:
    SpriteSession(std::shared_ptr<const SpriteTable>, Name, std::uint32_t first,
                  std::uint32_t count, std::uint64_t frameTicks, Playback);
    std::size_t frame() const;
    bool finished() const;
    void advance(std::uint64_t ticks);
    Checkpoint checkpoint() const { return state_; }
    void restore(Checkpoint);
    // Retains clip/mode/rate, resets phase; failed admission preserves all state.
    void reload(std::shared_ptr<const SpriteTable>);
    render::SpriteDrawPlan plan(std::int32_t x, std::int32_t y,
                               std::optional<render::SpriteColourTable> colours={}) const;
};
}
