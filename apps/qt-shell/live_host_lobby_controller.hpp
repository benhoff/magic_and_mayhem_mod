#pragma once
#include "live_menu_session.hpp"
#include "multiplayer_lobby_widget.hpp"
class LiveHostLobbyController final {
public:
    LiveHostLobbyController(LiveMenuSession&,MultiplayerLobbyWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;
    MultiplayerLobbyWidget& widget_;
};
