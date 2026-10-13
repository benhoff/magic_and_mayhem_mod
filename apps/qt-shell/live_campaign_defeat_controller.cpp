#include "live_campaign_defeat_controller.hpp"
#include "../../protocols/include/mnm/menu_v11.h"
LiveCampaignDefeatController::LiveCampaignDefeatController(LiveMenuSession& session,BattleResultWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&BattleResultWidget::continueRequested,&widget_,[this]{session_.requestDefeatContinue();});
}
bool LiveCampaignDefeatController::loadAssets(const QString& root,QString* error){
    if(!widget_.loadAssets(root,BattleResultWidget::Outcome::Defeat,error))return false;
    root_=root;return true;
}
bool LiveCampaignDefeatController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=MNM_MENU_DEFEAT_SCREEN)return true;
    if(!session_.campaignDefeatEnabled||state.defeat.context!=5||state.defeat.depth!=5||state.defeat.outcome>quint32(session_.progressedMenusEnabled)||state.defeat.actions!=1){if(error)*error="Unsupported campaign report; use original controls.";return false;}
    const auto outcome=state.defeat.outcome?BattleResultWidget::Outcome::Victory:BattleResultWidget::Outcome::Defeat;
    if(widget_.outcome()!=outcome&&!root_.isEmpty()&&!widget_.loadAssets(root_,outcome,error))return false;
    widget_.setOriginalReport(state.defeat.title,state.defeat.texts);return true;
}
