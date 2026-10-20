#include "session.hpp"
#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <stdexcept>

namespace mnm::scene {
namespace {
std::unique_ptr<game::MovementSession> makeSimulation(game::State state,
        const std::shared_ptr<const game::Navigation>& navigation) {
    if(!navigation) throw std::invalid_argument("Scene session requires owned navigation");
    game::World world(0);world.restore(std::move(state));
    return std::make_unique<game::MovementSession>(std::move(world),navigation);
}
}
SceneSession::SceneSession(game::State state,std::shared_ptr<const game::Navigation> navigation,
        std::function<std::uint64_t()> now,std::optional<std::uint64_t> geometryFingerprint)
    :navigation_(std::move(navigation)),simulation_(makeSimulation(std::move(state),navigation_)),
     playback_([this] {tick();},std::move(now)),
     geometryFingerprint_(geometryFingerprint.value_or(navigation_->binding().fingerprint)) {}
SceneSession::~SceneSession() {close();}
std::unique_ptr<SceneSession> SceneSession::load(const std::filesystem::path& path) {
    auto state=game::readSnapshot(path);
    if(!state.animation || !state.navigation)
        throw std::invalid_argument("Scene needs owned ANI and frozen navigation");
    auto navigation=sandbox::loadFrozenNavigation(state.map,state.animation);
    const auto geometry=navigation->binding().fingerprint;
    if(!(navigation->binding()==*state.navigation))
        navigation=sandbox::loadFrozenNavigation(state.map,state.animation,true);
    if(!(navigation->binding()==*state.navigation))
        navigation=sandbox::loadFrozenNavigation(state.map,state.animation,true,true);
    return std::make_unique<SceneSession>(std::move(state),std::move(navigation),
                                         std::function<std::uint64_t()>{},geometry);
}
game::MovementSession& SceneSession::simulation() {
    if(!active()) throw std::logic_error("Scene session is closed");
    return *simulation_;
}
const game::MovementSession& SceneSession::simulation() const {
    if(!active()) throw std::logic_error("Scene session is closed");
    return *simulation_;
}
const game::Navigation& SceneSession::navigation() const {
    if(!active()) throw std::logic_error("Scene session is closed");
    return *navigation_;
}
void SceneSession::tick() {
    if(!active()) throw std::logic_error("Scene session is closed");
    stepping_=true;
    try {simulation_->step();}
    catch(...) {stepping_=false;if(closing_) release();throw;}
    stepping_=false;
    if(closing_) {release();return;}
    const auto observer=afterTick;
    if(observer) observer();
}
void SceneSession::release() noexcept {
    orders_={};simulation_.reset();navigation_.reset();afterTick={};
}
void SceneSession::close() noexcept {
    closing_=true;playback_.retire();
    if(!stepping_) release();
}
}
