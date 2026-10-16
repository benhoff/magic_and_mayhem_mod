#pragma once
#include "arena.hpp"
#include "mission_presentation.hpp"
#include "playback.hpp"
#include "view.hpp"
namespace mnm::skirmish {
enum class CursorMode { select, summon, bolt, cure };
class Controller {
    game::SkirmishSession session_;
    std::unique_ptr<game::CampaignSession> campaign_;
    std::filesystem::path mapRoot_;
    std::shared_ptr<const MissionPresentation> mapPresentation_;
    scene::Playback playback_;
    std::optional<game::Handle> selected_;
    CursorMode mode_ = CursorMode::select;
    QString feedback_;
    game::SkirmishReport last_;
    void changed();
    void advance();
    void enqueue(game::SkirmishOrder);
    void adopted(const QString &);

  public:
    explicit Controller(std::filesystem::path mapRoot = {});
    bool hasMapRoot() const {return !mapRoot_.empty();}
    void newMapCampaign(std::filesystem::path root = {});
    const game::SkirmishSession &session() const;
    const game::CampaignSession *campaign() const { return campaign_.get(); }
    bool hasBattle() const { return !campaign_ || campaign_->battle(); }
    void newCampaign();
    void loadCampaign(const std::filesystem::path &);
    void saveCampaign(const std::filesystem::path &, bool overwrite = false);
    void continueCampaign();
    const game::SkirmishReport &lastReport() const { return last_; }
    ArenaView view() const;
    bool playing() const { return playback_.playing(); }
    CursorMode mode() const { return mode_; }
    void play();
    void pause();
    void step();
    void restart();
    void setMode(CursorMode);
    void cell(game::Point, bool right);
    void stop();
    void save(const std::filesystem::path &, bool overwrite = false);
    void load(const std::filesystem::path &);
    std::function<void()> onChanged, afterTick;
    std::function<void(const game::SkirmishOrder &)> onOrder;
};
} // namespace mnm::skirmish
