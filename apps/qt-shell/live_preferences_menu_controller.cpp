#include "live_preferences_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v6.h"
LivePreferencesMenuController::LivePreferencesMenuController(LiveMenuSession& session,PreferencesWidget& widget):session_(session),widget_(widget){
    QObject::connect(&widget_,&PreferencesWidget::audioLevelChanged,&widget_,[this](int channel,int){if(entered_&&!closing_&&channel>=0&&channel<2)dirty_[channel]=true;});
    QObject::connect(&widget_,&PreferencesWidget::settingsApplied,&widget_,[this]{if(entered_&&!closing_){button_=MNM_MENU_PREFERENCES_OK;closing_=true;pump();}});
    QObject::connect(&widget_,&PreferencesWidget::cancelled,&widget_,[this]{if(entered_&&!closing_){button_=MNM_MENU_PREFERENCES_CANCEL;closing_=true;pump();}});
    timer_.setInterval(50);QObject::connect(&timer_,&QTimer::timeout,&widget_,[this]{pump();});timer_.start();
}
std::array<int,7> LivePreferencesMenuController::values() const{
    const auto b=widget_.draftSettings();return {b.musicLevel,b.soundLevel,int(b.resolution),int(b.animation),int(b.dialogueSpeed),int(b.gameSpeed),b.borderPicture?1:0};
}
void LivePreferencesMenuController::pump(){
    if(!entered_)return;
    if(button_){if(session_.requestPreferences(button_,values())){button_=0;dirty_={};}return;}
    if(closing_)return;
    for(int i=0;i<2;++i)if(dirty_[i]){if(session_.requestPreferences(MNM_MENU_PREFERENCES_PREVIEW,values(),i))dirty_[i]=false;return;}
}
bool LivePreferencesMenuController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=MNM_MENU_PREFERENCES_SCREEN){entered_=closing_=false;dirty_={};button_=0;return true;}
    if(!state.ready)return true;
    const auto& b=state.preferences;if(b.parentScreen!=3){if(error)*error="Unsupported Preferences caller.";return false;}
    PreferencesWidget::ControlPolicy policy;
    for(int i=0;i<12;++i)policy.radios[i]=b.available&(1u<<i);
    for(int i=0;i<2;++i)policy.sliders[i]=b.available&(1u<<(12+i));
    policy.canApply=b.actions&1;policy.canCancel=b.actions&2;
    if(!entered_){const auto& v=b.values;PreferencesWidget::Settings settings;
        settings.musicLevel=v[0];settings.soundLevel=v[1];settings.resolution=PreferencesWidget::Resolution(v[2]);settings.animation=PreferencesWidget::Animation(v[3]);
        settings.dialogueSpeed=PreferencesWidget::Speed(v[4]);settings.gameSpeed=PreferencesWidget::Speed(v[5]);settings.borderPicture=v[6];
        if(!widget_.setEngineSettings(settings,policy,error))return false;
        entered_=true;
    }else widget_.setControlAvailability(policy); // Preserve radio/audio drafts across acknowledgements.
    return true;
}
