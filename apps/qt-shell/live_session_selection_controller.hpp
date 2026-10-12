#pragma once
#include "live_menu_session.hpp"
#include "multiplayer_game_selection_widget.hpp"
class LiveSessionSelectionController final {
public:
    LiveSessionSelectionController(LiveMenuSession&,MultiplayerGameSelectionWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    LiveMenuSession& session_;
    MultiplayerGameSelectionWidget& widget_;
    bool entered_=false;
    QVector<QString> names_;
};
