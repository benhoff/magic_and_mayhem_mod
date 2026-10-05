#include "live_battle_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v2.h"
#include <QPushButton>
LiveBattleMenuController::LiveBattleMenuController(LiveMenuSession& session,SinglePlayerBattleWidget& setup,MapSelectionWidget& maps):session_(session),setup_(setup),maps_(maps){
    QObject::connect(&setup_,&SinglePlayerBattleWidget::cancelled,&setup_,[this]{session_.request(MNM_MENU_SETUP_CANCEL);});
    QObject::connect(&setup_,&SinglePlayerBattleWidget::mapRequested,&setup_,[this]{auto rules=draft();session_.request(MNM_MENU_SETUP_MAP,0,&rules);});
    QObject::connect(&setup_,&SinglePlayerBattleWidget::startRequested,&setup_,[this]{auto rules=draft();session_.request(MNM_MENU_SETUP_START,0,&rules);});
    QObject::connect(&setup_,&SinglePlayerBattleWidget::playerChangeRequested,&setup_,[this](int slot){if(slot==0){auto rules=draft();session_.request(MNM_MENU_SETUP_PLAYER,0,&rules);}});
    QObject::connect(&setup_,&SinglePlayerBattleWidget::colourChangeRequested,&setup_,[this](int slot){if(slot==0){auto rules=draft();session_.request(MNM_MENU_SETUP_PLAYER,1,&rules);}});
    QObject::connect(&setup_,&SinglePlayerBattleWidget::playerRemovalRequested,&setup_,[this](int slot){if(slot==2||slot==3){auto rules=draft();session_.request(MNM_MENU_SETUP_PLAYER,slot,&rules);}});
    QObject::connect(&maps_,&MapSelectionWidget::cancelled,&maps_,[this]{session_.request(MNM_MENU_MAP_CANCEL);});
    QObject::connect(&maps_,&MapSelectionWidget::mapSelected,&maps_,[this](const QString& id){bool ok=false;auto ordinal=id.toUInt(&ok);if(ok)session_.request(MNM_MENU_MAP_OK,ordinal);});
}
std::array<int,17> LiveBattleMenuController::draft() const{
    const auto model=setup_.setup();std::array<int,17> rules;
    for(int i=0;i<13;++i)rules[i]=model.values[i];
    for(int i=0;i<4;++i)rules[13+i]=model.players[i].handicap;
    return rules;
}
bool LiveBattleMenuController::loadAssets(const QString& root,QString* error){return setup_.loadAssets(root,error)&&maps_.loadAssets(root,error);}
bool LiveBattleMenuController::present(const MenuBridge::State& state,QString* error){
    if(!state.ready||state.generation==generation_||(state.screen!=14&&state.screen!=25))return true;
    const auto& data=state.battle;
    if(state.screen==14){
        SinglePlayerBattleWidget::Setup model;model.values=data.rules;
        if(data.map){model.mapId=QString::number(data.map);model.mapName=data.mapName;}
        for(int i=0;i<4;++i){auto& p=model.players[i];const auto& engine=data.players[i];
            p.active=engine.active;p.name=engine.name;p.handicap=engine.handicap;
            if(p.active){p.portraitIndex=engine.portrait;p.colourIndex=engine.colour;
                p.portraitId=QString::number(engine.portrait);p.portraitText=engine.name;
                p.colourId=QString::number(engine.colour);p.colourText=QString("Colour %1").arg(engine.colour+1);}
        }
        if(!setup_.setEngineSetup(model,error))return false;
        // Original opponent portraits/colours are pictures, not editable buttons.
        for(int i=1;i<4;++i){setup_.findChild<QPushButton*>(QString("singlePlayerPortrait%1").arg(i))->setEnabled(false);setup_.findChild<QPushButton*>(QString("singlePlayerColour%1").arg(i))->setEnabled(false);}
    }else{
        QVector<MapSelectionWidget::Map> maps;
        for(int i=0;i<data.maps.size();++i)maps.append({QString::number(i+1),data.maps[i]});
        if(maps.isEmpty()||!maps_.setMaps(maps,data.map?QString::number(data.map):QString(),error))return false;
    }
    generation_=state.generation;return true;
}
