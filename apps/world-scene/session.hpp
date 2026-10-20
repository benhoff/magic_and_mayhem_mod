#pragma once
#include "orders.hpp"
#include "playback.hpp"
#include <filesystem>
#include <memory>

namespace mnm::scene {
// Application lifetime owner, confined to its Qt thread. It contains no widgets,
// renderer, process bridge or original-game pointers. Restart creates a new owner.
class SceneSession final {
    std::shared_ptr<const game::Navigation> navigation_;
    std::unique_ptr<game::MovementSession> simulation_;
    Orders orders_;
    Playback playback_;
    std::uint64_t geometryFingerprint_;
    bool stepping_=false, closing_=false;
    void tick();
    void release() noexcept;
public:
    SceneSession(game::State,std::shared_ptr<const game::Navigation>,
                 std::function<std::uint64_t()> now={},std::optional<std::uint64_t> geometryFingerprint={});
    ~SceneSession();
    SceneSession(const SceneSession&)=delete;
    SceneSession& operator=(const SceneSession&)=delete;
    static std::unique_ptr<SceneSession> load(const std::filesystem::path&);
    bool active() const {return simulation_ && !closing_;}
    game::MovementSession& simulation();
    const game::MovementSession& simulation() const;
    const game::Navigation& navigation() const;
    std::uint64_t geometryFingerprint() const {return geometryFingerprint_;}
    Orders& orders() {return orders_;}
    Playback& playback() {return playback_;}
    // Idempotent terminal close. An in-flight simulation step finishes before
    // releasing its storage; all further tick admission is already retired.
    void close() noexcept;
    std::function<void()> afterTick;
};
}
