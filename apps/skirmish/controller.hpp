#pragma once
#include "arena.hpp"
#include "view.hpp"
#include "playback.hpp"
namespace mnm::skirmish {
enum class CursorMode { select, summon, bolt };
class Controller {
    game::SkirmishSession session_;
    scene::Playback playback_;
    std::optional<game::Handle> selected_;
    CursorMode mode_ = CursorMode::select;
    QString feedback_;
    game::SkirmishReport last_;
    void changed();
    void advance();

  public:
    Controller();
    const game::SkirmishSession &session() const { return session_; }
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
