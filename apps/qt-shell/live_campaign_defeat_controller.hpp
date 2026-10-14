#pragma once
#include "live_menu_session.hpp"
#include "battle_result_widget.hpp"
class LiveCampaignDefeatController final {
public:
    LiveCampaignDefeatController(LiveMenuSession&,BattleResultWidget&);
    bool loadAssets(const QString&,QString* error=nullptr);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;BattleResultWidget& widget_;
    QString root_;
};
