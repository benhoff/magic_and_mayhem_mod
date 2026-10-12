#pragma once
#include "character_screen_widget.hpp"
#include "live_menu_session.hpp"
class LiveCharacterController final {
public:
    LiveCharacterController(LiveMenuSession&,CharacterScreenWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;
    CharacterScreenWidget& widget_;
    quint32 generation_=0;
};
