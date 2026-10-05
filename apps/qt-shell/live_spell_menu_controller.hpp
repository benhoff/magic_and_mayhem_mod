#pragma once
#include "live_menu_session.hpp"
#include "spellbox_widget.hpp"
class LiveSpellMenuController final {
public:
    LiveSpellMenuController(LiveMenuSession&,SpellboxWidget&);
    bool present(const MenuBridge::State&,QString* error);
private:
    LiveMenuSession& session_;SpellboxWidget& widget_;
    SpellboxWidget::Inventory inventory_;quint32 generation_=0;
};
