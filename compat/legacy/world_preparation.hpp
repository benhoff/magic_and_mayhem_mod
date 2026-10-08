#pragma once
#include "world_resources.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <variant>

namespace mnm::legacy {
struct WorldPreparationPlan {
    std::shared_ptr<const WorldFrame> frame;
    std::vector<WorldAssetFile> files;
};
struct PreparedWorld {
    std::shared_ptr<const WorldFrame> frame;
    std::vector<assets::PreparedResource> resources;
    std::vector<render::SceneDraw> draws;
};
struct PreparedWorldUpload {
    std::shared_ptr<const WorldFrame> frame;
    render::SceneUploadNeed need;
    render::PreparedSpriteFrame planes;
};
using WorldPreparationReply=std::variant<WorldPreparationPlan,PreparedWorld,PreparedWorldUpload,std::string>;
struct WorldPreparationStats {
    std::uint64_t catalogueMs=0,prepareMs=0,resourcesPrepared=0,uploadPrepareMs=0,uploadsPrepared=0,retainedDecodedBytes=0;
};
// Optional CPU checkpoint for deterministic I/O scheduling/cancellation tests.
// Captures must outlive outstanding work; callback receives the stop flag.
using WorldPreparationCheckpoint=std::function<void(const std::atomic<bool>&)>;
// One worker, one outstanding job and one completion. State owns all worker
// inputs. Cancellation never joins file I/O on the GUI thread, and closed state
// cannot publish or access widgets/managers. The worker releases its state on exit.
class WorldPreparation final {
public:
    explicit WorldPreparation(assets::AssetStore,WorldPreparationCheckpoint={});
    ~WorldPreparation();
    WorldPreparation(const WorldPreparation&)=delete;
    WorldPreparation& operator=(const WorldPreparation&)=delete;
    void plan(QByteArray inputs);
    void prepare(std::vector<assets::ResourcePreparation>,std::uint64_t availableBytes,std::size_t availableResources);
    void upload(render::SceneUploadNeed);
    std::optional<WorldPreparationReply> take();
    WorldPreparationStats stats() const;
    void cancel();
    bool stopped() const;
private:
    struct State;
    std::shared_ptr<State> state_;
    static void run(const std::shared_ptr<State>&);
};
}
