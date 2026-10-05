#pragma once
#include "live_menu_session.hpp"
#include "region_entry_widget.hpp"
class LiveRegionEntryController final {
public:
    LiveRegionEntryController(LiveMenuSession&,RegionEntryWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    void pump();
    LiveMenuSession& session_;RegionEntryWidget& widget_;QTimer timer_;
    bool entered_=false,closing_=false,dirty_=false;
    quint32 draft_=0,action_=0;
};
