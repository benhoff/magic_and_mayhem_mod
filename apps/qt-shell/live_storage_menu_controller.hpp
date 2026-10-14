#pragma once
#include "live_menu_session.hpp"
#include "load_game_widget.hpp"
#include "save_game_widget.hpp"
class LiveStorageMenuController final {
public:
    LiveStorageMenuController(LiveMenuSession&,LoadGameWidget&,SaveGameWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;
    LoadGameWidget& load_;
    SaveGameWidget& save_;
    quint32 generation_=0;
};
