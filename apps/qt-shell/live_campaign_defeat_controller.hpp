#pragma once
#include "live_menu_session.hpp"
#include "battle_result_widget.hpp"
class LiveCampaignDefeatController final {
public:
    LiveCampaignDefeatController(LiveMenuSession&,BattleResultWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;BattleResultWidget& widget_;
};
