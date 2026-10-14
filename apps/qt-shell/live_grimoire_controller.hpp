#pragma once
#include "live_menu_session.hpp"
#include "grimoire_widget.hpp"
class LiveGrimoireController final {
public:
    LiveGrimoireController(LiveMenuSession&,GrimoireWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    void pump();
    LiveMenuSession& session_;GrimoireWidget& widget_;QTimer timer_;
    bool entered_=false,pending_=false;quint32 action_=0,section_=0;
};
