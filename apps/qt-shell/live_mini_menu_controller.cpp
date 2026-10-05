#include "live_mini_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v4.h"
#include <QPushButton>
LiveMiniMenuController::LiveMiniMenuController(LiveMenuSession& session,MiniMenuWidget& widget):session_(session),widget_(widget){
    widget_.setMode(MiniMenuWidget::Mode::Battle);
    QObject::connect(&widget_,&MiniMenuWidget::actionRequested,&widget_,[this](MiniMenuWidget::Action action){
        if(action==MiniMenuWidget::Action::Cancel)session_.requestMini(MNM_MENU_MINI_CANCEL);
        else if(action==MiniMenuWidget::Action::Preferences)session_.requestMini(MNM_MENU_MINI_PREFERENCES);
        else if(action==MiniMenuWidget::Action::QuitBattle)session_.requestMini(MNM_MENU_MINI_QUIT);
    });
}
bool LiveMiniMenuController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=MNM_MENU_MINI_SCREEN)return true;
    if(!session_.miniMenusEnabled()||!state.mini.battle){if(error)*error="Unsupported Mini Menu context; use original controls.";return false;}
    const quint32 bits[]={MNM_MENU_MINI_CAN_PREFERENCES,MNM_MENU_MINI_CAN_QUIT,MNM_MENU_MINI_CAN_CANCEL};
    for(int i=0;i<3;++i)widget_.findChild<QPushButton*>(QString("miniMenuButton%1").arg(i+6))->setEnabled(state.ready&&(state.mini.actions&bits[i]));
    return true;
}
