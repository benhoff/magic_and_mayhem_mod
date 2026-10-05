#include "live_preferences_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v6.h"
#include <QApplication>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QKeyEvent>
#include <cstdio>
#include <cstdlib>
static void check(bool ok,int line){if(!ok){std::fprintf(stderr,"Preferences controller failed at %d\n",line);std::exit(1);}}
#define require(ok) check((ok),__LINE__)
int main(int argc,char** argv){
    QApplication app(argc,argv);LiveMenuSession session("/unused");PreferencesWidget widget;
    LivePreferencesMenuController controller(session,widget);MenuBridge::State state;
    state.screen=10;state.ready=1;state.preferences={3,16383,3,3,{0,-250,0,0,1,0,0}};
    int preview=0,applied=0;QObject::connect(&widget,&PreferencesWidget::audioLevelChanged,[&](int,int){++preview;});
    QObject::connect(&widget,&PreferencesWidget::settingsApplied,[&](const auto&){++applied;});
    require(controller.present(state)&&preview==0);widget.show();app.processEvents();
    auto* music=widget.findChild<QSlider*>("preferencesSlider1");auto* fx=widget.findChild<QSlider*>("preferencesSlider2");
    require(music&&fx&&music->minimum()==0&&music->maximum()==15&&music->singleStep()==1&&fx->minimum()==-2500&&fx->maximum()==0&&fx->singleStep()==50);
    auto* slow=widget.findChild<QRadioButton*>("preferencesRadio7");slow->click();fx->setValue(-500);require(preview==1);
    state.preferences.values[1]=-500;++state.generation;require(controller.present(state));
    require(widget.draftSettings().dialogueSpeed==PreferencesWidget::Speed::Slow&&fx->value()==-500&&preview==1);
    state.preferences.available&=~(1u<<12);state.preferences.actions=2;require(controller.present(state));widget.focusFirstControl();
    require(!music->isEnabled()&&fx->hasFocus()&&!widget.findChild<QPushButton*>("preferencesOk")->isEnabled());
    QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);QApplication::sendEvent(fx,&enter);require(applied==0);
    widget.findChild<QPushButton*>("preferencesCancel")->click();require(widget.draftSettings().soundLevel==-250&&preview==1);
    state.screen=3;require(controller.present(state));state.screen=10;state.preferences.actions=3;state.preferences.values[4]=0;
    require(controller.present(state)&&widget.draftSettings().dialogueSpeed==PreferencesWidget::Speed::Fast&&fx->value()==-500&&preview==1);
    auto invalid=widget.settings();invalid.musicLevel=16;PreferencesWidget::ControlPolicy policy;
    require(!widget.setEngineSettings(invalid,policy)&&widget.settings().musicLevel==0&&fx->value()==-500);
    state.screen=3;require(controller.present(state));state.screen=10;state.preferences.parentScreen=27;QString error;
    require(!controller.present(state,&error)&&!error.isEmpty());
    std::puts("Preferences controller checks passed");
}
