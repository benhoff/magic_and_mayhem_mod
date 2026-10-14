#include "live_multiplayer_controller.hpp"
#include "../../protocols/include/mnm/menu_v15.h"
LiveMultiplayerController::LiveMultiplayerController(LiveMenuSession& session,MultiplayerSetupWidget& create,MultiplayerSetupWidget& join):session_(session),create_(create),join_(join){
    for(auto* w:{&create_,&join_}){
        QObject::connect(w,&MultiplayerSetupWidget::requestSubmitted,w,[this](const auto& r){if(session_.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,quint32(r.transport),r.gameName,r.userName))entered_=0;});
        QObject::connect(w,&MultiplayerSetupWidget::cancelled,w,[this]{session_.requestMultiplayer(MNM_MENU_MULTIPLAYER_CANCEL);});
    }
}
bool LiveMultiplayerController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=15&&state.screen!=16){entered_=0;return true;}
    if(!state.ready)return true;
    auto& w=state.screen==15?create_:join_;const auto& b=state.multiplayer;
    if(b.parent!=22||b.depth!=4||b.context!=2||b.transport>2||b.modal||b.pending||b.userLimit!=15||b.gameLimit!=(state.screen==15?256u:0u)){if(error)*error="Unsupported multiplayer setup caller.";return false;}
    w.setLiveAvailability(b.available,b.actions);
    // Idle refreshes retain the user's draft; a new admission uses original text.
    if(entered_!=state.screen&&!w.setForm({b.gameName,b.userName,MultiplayerSetupWidget::Transport(b.transport)},error))return false;
    entered_=state.screen;return true;
}
