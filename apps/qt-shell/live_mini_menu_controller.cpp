#include "live_mini_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v23.h"
#include <QMessageBox>
#include <QPushButton>
LiveMiniMenuController::LiveMiniMenuController(LiveMenuSession& session,MiniMenuWidget& widget):session_(session),widget_(widget){
    widget_.setMode(MiniMenuWidget::Mode::Battle);
    QObject::connect(&widget_,&MiniMenuWidget::actionRequested,&widget_,[this](MiniMenuWidget::Action action){
        if(action==MiniMenuWidget::Action::RestartLevel){
            if(confirming_||!widget_.isEnabled()||!widget_.findChild<QPushButton*>("miniMenuRestart")->isEnabled())return;
            confirming_=true;const auto generation=generation_;
            auto* dialog=new QMessageBox(QMessageBox::Question,"Restart Level",
                "Restart this level from the beginning? Progress made on this level will be lost.",
                QMessageBox::Yes|QMessageBox::No,&widget_);
            dialog->setObjectName("restartLevelConfirmation");
            dialog->setDefaultButton(QMessageBox::No);
            dialog->setEscapeButton(QMessageBox::No);
            dialog->setAttribute(Qt::WA_DeleteOnClose);
            QObject::connect(dialog,&QMessageBox::finished,&widget_,[this,generation](int answer){
                confirming_=false;
                if(answer==QMessageBox::Yes&&generation_==generation&&widget_.isEnabled())session_.requestMini(MNM_MENU_MINI_RESTART);
            });
            dialog->open();return;
        }
        if(action==MiniMenuWidget::Action::LoadGame)session_.requestMini(MNM_MENU_MINI_LOAD);
        else if(action==MiniMenuWidget::Action::SaveGame)session_.requestMini(MNM_MENU_MINI_SAVE);
        else if(action==MiniMenuWidget::Action::Cancel)session_.requestMini(MNM_MENU_MINI_CANCEL);
        else if(action==MiniMenuWidget::Action::Preferences)session_.requestMini(MNM_MENU_MINI_PREFERENCES);
        else if((action==MiniMenuWidget::Action::QuitBattle||action==MiniMenuWidget::Action::QuitGame))session_.requestMini(MNM_MENU_MINI_QUIT);
    });
}
bool LiveMiniMenuController::present(const MenuBridge::State& state,QString* error){
    generation_=state.generation;
    widget_.setRestartAvailable(state.screen==MNM_MENU_MINI_SCREEN&&state.ready&&!state.mini.battle&&
        (state.mini.actions&MNM_MENU_MINI_CAN_RESTART),session_.campaignRestartEnabled&&session_.storageMenusEnabled);
    if(state.screen!=MNM_MENU_MINI_SCREEN)return true;
    if(!session_.miniMenusEnabled()||(!state.mini.battle&&(!session_.campaignMiniEnabled||state.mini.context!=5||state.mini.mode!=2))){if(error)*error="Unsupported Mini Menu context; use original controls.";return false;}
    const auto mode=state.mini.battle?MiniMenuWidget::Mode::Battle:MiniMenuWidget::Mode::Campaign;
    if(widget_.mode()!=mode)widget_.setMode(mode);
    if(!state.mini.battle){for(int i=1;i<=5;++i)widget_.findChild<QPushButton*>(QString("miniMenuButton%1").arg(i))->setEnabled(state.ready&&((i==1&&session_.storageMenusEnabled&&(state.mini.actions&MNM_MENU_MINI_CAN_LOAD))||(i==2&&session_.storageMenusEnabled&&(state.mini.actions&MNM_MENU_MINI_CAN_SAVE))||(i==3&&session_.campaignPreferencesEnabled&&(state.mini.actions&MNM_MENU_MINI_CAN_PREFERENCES))||(i==5&&(state.mini.actions&MNM_MENU_MINI_CAN_CANCEL))||(i==4&&session_.campaignQuitEnabled&&(state.mini.actions&MNM_MENU_MINI_CAN_QUIT))));return true;}
    const quint32 bits[]={MNM_MENU_MINI_CAN_PREFERENCES,MNM_MENU_MINI_CAN_QUIT,MNM_MENU_MINI_CAN_CANCEL};
    for(int i=0;i<3;++i)widget_.findChild<QPushButton*>(QString("miniMenuButton%1").arg(i+6))->setEnabled(state.ready&&(state.mini.actions&bits[i]));
    return true;
}
