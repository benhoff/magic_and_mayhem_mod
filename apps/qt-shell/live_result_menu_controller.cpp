#include "live_result_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v5.h"
LiveResultMenuController::LiveResultMenuController(LiveMenuSession& session,QuickBattleResultWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&QuickBattleResultWidget::actionRequested,&widget_,[this](QuickBattleResultWidget::Action action){
        if(action==QuickBattleResultWidget::Action::Continue)session_.requestResults(MNM_MENU_RESULT_CONTINUE);
        else if(action==QuickBattleResultWidget::Action::Quit)session_.requestResults(MNM_MENU_RESULT_QUIT);
    });
}
bool LiveResultMenuController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=MNM_MENU_RESULT_SCREEN)return true;
    if(state.results.context!=1){if(error)*error="Unsupported results context; use original controls.";return false;}
    QuickBattleResultWidget::Results model;
    model.primaryAction=state.results.actions&1?QuickBattleResultWidget::PrimaryAction::Continue:QuickBattleResultWidget::PrimaryAction::None;
    model.canQuit=state.results.actions&2;
    for(int i=0;i<4;++i){const auto& p=state.results.players[i];auto& row=model.players[i];
        row.active=p.active;row.name=p.name;row.portraitText=p.name.left(1);row.kills=p.kills;row.deaths=p.deaths;row.handicapBonus=p.handicap;row.score=p.score;
    }
    widget_.setResults(model);return true;
}
