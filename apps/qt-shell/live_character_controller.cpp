#include "live_character_controller.hpp"
#include "../../protocols/include/mnm/menu_v18.h"
LiveCharacterController::LiveCharacterController(LiveMenuSession& session,CharacterScreenWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&CharacterScreenWidget::adjustmentRequested,&widget_,[this](auto attribute,bool up){session_.request(MNM_MENU_CHARACTER_ADJUST,quint32(attribute)*2+quint32(up));});
    QObject::connect(&widget_,&CharacterScreenWidget::characterAccepted,&widget_,[this](const auto&){session_.request(MNM_MENU_CHARACTER_OK);});
    QObject::connect(&widget_,&CharacterScreenWidget::cancelled,&widget_,[this]{session_.request(MNM_MENU_CHARACTER_CANCEL);});
}
bool LiveCharacterController::present(const MenuBridge::State& state,QString* error){
    widget_.setEnabled(state.screen==20&&state.ready&&session_.characterMenusEnabled);
    if(state.screen!=20){generation_=0;return true;}
    if(!session_.characterMenusEnabled){if(error)*error="Character integration disabled.";return false;}
    if(!state.ready||state.generation==generation_)return true;
    const auto& b=state.character;CharacterScreenWidget::Character c;
    c.id=QString("original-wizard-%1").arg(b.wizard);c.name=b.name;c.experiencePoints=int(b.experience);
    std::array<int,6> opening{},decrease{};
    for(int i=0;i<6;++i){const auto& row=b.rows[i];opening[i]=int(row.opening);decrease[i]=int(row.decrease);
        c.stats[i].value=int(row.value);c.stats[i].upgradeAvailable=b.adjustments&(1u<<(i*2+1));
        if(row.value+row.increment<=row.maximum)c.stats[i].upgradeCosts.append(int(row.increase));
    }
    if(!widget_.setLiveCharacter(c,opening,decrease,b.adjustments,error))return false;
    generation_=state.generation;return true;
}
