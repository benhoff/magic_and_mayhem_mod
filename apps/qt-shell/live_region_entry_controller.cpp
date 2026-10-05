#include "live_region_entry_controller.hpp"
#include "../../protocols/include/mnm/menu_v8.h"
LiveRegionEntryController::LiveRegionEntryController(LiveMenuSession& session,RegionEntryWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&RegionEntryWidget::difficultyChanged,&widget_,[this](auto difficulty){if(entered_&&!closing_){draft_=quint32(difficulty);dirty_=true;pump();}});
    QObject::connect(&widget_,&RegionEntryWidget::cancelled,&widget_,[this]{if(entered_&&!closing_){closing_=true;dirty_=false;action_=MNM_MENU_REGION_CANCEL;pump();}});
    QObject::connect(&widget_,&RegionEntryWidget::enterRequested,&widget_,[this]{if(entered_&&!closing_){closing_=true;action_=MNM_MENU_REGION_ENTER;pump();}});
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
    if(b.caller!=4||b.depth!=4||b.different||b.region!=1||b.realm!="Celtic"||b.difficulty>3||b.name.isEmpty()){
        if(error)*error="Unsupported Region Entry caller or region.";
        return false;
    }
    if(!entered_&&!closing_){
        RegionEntryWidget::Region region;region.id="Celtic:1";region.name=b.name;region.difficulty=RegionEntryWidget::Difficulty(b.difficulty);
        region.enterAvailable=b.actions&2;region.auxiliaryAvailable={false,false,false};
        if(!widget_.setRegion(region,error))return false;
        entered_=true;
    }else if(!dirty_&&!closing_)widget_.setDifficulty(RegionEntryWidget::Difficulty(b.difficulty));
    widget_.setControlAvailability(b.available,b.actions&1,b.actions&2);return true;
}
