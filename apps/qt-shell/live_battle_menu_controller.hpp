#pragma once
#include "live_menu_session.hpp"
#include "single_player_battle_widget.hpp"
#include "map_selection_widget.hpp"
// Application adapter: translate authoritative wire snapshots to widget models.
// Presentation widgets know no game addresses, process state, or wire layout.
class LiveBattleMenuController final {
public:
    LiveBattleMenuController(LiveMenuSession&,SinglePlayerBattleWidget&,MapSelectionWidget&);
    bool loadAssets(const QString& root,QString* error);
    bool present(const MenuBridge::State&,QString* error);
private:
    std::array<int,17> draft() const;
    LiveMenuSession& session_;
    SinglePlayerBattleWidget& setup_;
    MapSelectionWidget& maps_;
    quint32 generation_=0;
};
