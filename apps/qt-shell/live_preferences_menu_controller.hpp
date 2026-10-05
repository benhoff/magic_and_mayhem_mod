#pragma once
#include "live_menu_session.hpp"
#include "preferences_widget.hpp"
class LivePreferencesMenuController final {
public:
    LivePreferencesMenuController(LiveMenuSession&,PreferencesWidget&);
    bool present(const MenuBridge::State&,QString* error=nullptr);
private:
    void pump();
    std::array<int,7> values() const;
    LiveMenuSession& session_;PreferencesWidget& widget_;QTimer timer_;
    bool entered_=false,closing_=false;
    std::array<bool,2> dirty_{};quint32 button_=0;
};
