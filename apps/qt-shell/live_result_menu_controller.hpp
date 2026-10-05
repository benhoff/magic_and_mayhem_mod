#pragma once
#include "live_menu_session.hpp"
#include "quick_battle_result_widget.hpp"
class LiveResultMenuController final {
public:
    LiveResultMenuController(LiveMenuSession&,QuickBattleResultWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;QuickBattleResultWidget& widget_;
};
