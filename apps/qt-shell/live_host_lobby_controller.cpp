#include "live_host_lobby_controller.hpp"
#include "../../protocols/include/mnm/menu_v17.h"
LiveHostLobbyController::LiveHostLobbyController(LiveMenuSession& session,MultiplayerLobbyWidget& widget):session_(session),widget_(widget){widget_.setLiveViewOnly(true);QObject::connect(&widget_,&MultiplayerLobbyWidget::cancelled,&widget_,[this]{session_.request(MNM_MENU_LOBBY_CANCEL);});}
bool LiveHostLobbyController::present(const MenuBridge::State& state,QString* error){
 if(state.screen!=24||!state.ready)return true;
 const auto& b=state.lobby;MultiplayerLobbyWidget::Lobby model;
 model.sessionId="original-current-host-room";model.gameName=b.gameName;model.mapId=QString::number(b.map);model.mapName=b.mapName.isEmpty()?QString("No Map"):b.mapName;model.values=b.rules;
 for(int i=0;i<4;++i){auto& out=model.players[i];const auto& player=b.players[i];out.active=player.active;out.name=player.name;out.handicap=player.handicap;if(player.active){out.portraitId=QString::number(player.portrait);out.portraitText=out.portraitId;out.colourId=QString::number(player.colour);out.colourText=out.colourId;out.portraitIndex=player.portrait;out.colourIndex=player.colour;}}
 return widget_.setLobby(model,error);
}
