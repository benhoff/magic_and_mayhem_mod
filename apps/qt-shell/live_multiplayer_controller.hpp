#pragma once
#include "live_menu_session.hpp"
#include "multiplayer_setup_widget.hpp"
class LiveMultiplayerController final {
public:
    LiveMultiplayerController(LiveMenuSession&,MultiplayerSetupWidget&,MultiplayerSetupWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;
    MultiplayerSetupWidget& create_;
    MultiplayerSetupWidget& join_;
    quint32 entered_=0;
};
