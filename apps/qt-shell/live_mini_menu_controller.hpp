#pragma once
#include "live_menu_session.hpp"
#include "mini_menu_widget.hpp"
class LiveMiniMenuController final {
public:
    LiveMiniMenuController(LiveMenuSession&,MiniMenuWidget&);
    bool present(const MenuBridge::State&,QString* error);
private:
    LiveMenuSession& session_;MiniMenuWidget& widget_;
};
