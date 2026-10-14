#include "live_region_entry_controller.hpp"
#include "../../protocols/include/mnm/menu_v20.h"
LiveRegionEntryController::LiveRegionEntryController(LiveMenuSession& session,RegionEntryWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&RegionEntryWidget::difficultyChanged,&widget_,[this](auto difficulty){if(entered_&&!closing_){draft_=quint32(difficulty);dirty_=true;pump();}});
    QObject::connect(&widget_,&RegionEntryWidget::cancelled,&widget_,[this]{if(entered_&&!closing_){closing_=true;dirty_=false;action_=MNM_MENU_REGION_CANCEL;pump();}});
    QObject::connect(&widget_,&RegionEntryWidget::enterRequested,&widget_,[this]{if(entered_&&!closing_){closing_=true;action_=MNM_MENU_REGION_ENTER;pump();}});
    QObject::connect(&widget_,&RegionEntryWidget::auxiliaryRequested,&widget_,[this](auto action){if(entered_&&!closing_){
        if(action==RegionEntryWidget::AuxiliaryAction::Grimoire)action_=MNM_MENU_REGION_GRIMOIRE;
        else if(session_.progressedMenusEnabled&&action==RegionEntryWidget::AuxiliaryAction::Character)action_=MNM_MENU_REGION_CHARACTER;
        else if(session_.progressedMenusEnabled&&action==RegionEntryWidget::AuxiliaryAction::Spellbox)action_=MNM_MENU_REGION_SPELLBOX;
        else return;
        closing_=true;pump();
    }});
    timer_.setInterval(50);QObject::connect(&timer_,&QTimer::timeout,&widget_,[this]{pump();});timer_.start();
}
void LiveRegionEntryController::pump(){
    if(!entered_)return;
    if(dirty_){if(session_.request(MNM_MENU_REGION_DIFFICULTY,draft_))dirty_=false;return;}
    if(closing_){if(session_.request(action_))entered_=false;return;}
}
bool LiveRegionEntryController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=18){entered_=closing_=dirty_=false;return true;}
    if(!state.ready)return true;
    const auto& b=state.region;
    const bool broader=session_.progressedMenusEnabled;
    if(b.caller!=4||b.depth!=4||b.different>1||(!broader&&b.different)||!b.region||b.region>19||(!broader&&b.region!=1)||
       (broader?(b.realm!="Celtic"&&b.realm!="Greek"&&b.realm!="Medieval"):b.realm!="Celtic")||b.difficulty>3||b.name.isEmpty()){
        if(error)*error="Unsupported Region Entry caller or region.";
        return false;
    }
    // The session presents a ready caller again only after accepting its fresh
    // engine return. Auxiliary handoff can leave without presenting screen0.
    if(!entered_)closing_=false;
    if(!entered_&&!closing_){
        RegionEntryWidget::Region region;region.id=QString("%1:%2").arg(b.realm).arg(b.region);region.name=b.name;region.difficulty=RegionEntryWidget::Difficulty(b.difficulty);
        region.artworkRealm=b.realm=="Greek"?RegionEntryWidget::Realm::Greek:b.realm=="Medieval"?RegionEntryWidget::Realm::Medieval:RegionEntryWidget::Realm::Celtic;region.artworkNumber=int(b.region);
        region.enterAvailable=b.actions&2;region.auxiliaryAvailable={bool(b.actions&4),broader&&bool(b.actions&8),broader&&bool(b.actions&16)};
        if(!widget_.setRegion(region,error))return false;
        entered_=true;
    }else if(!dirty_&&!closing_)widget_.setDifficulty(RegionEntryWidget::Difficulty(b.difficulty));
    widget_.setControlAvailability(b.available,b.actions&1,b.actions&2);return true;
}
