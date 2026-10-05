#include "live_result_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v5.h"
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <cstdio>
#include <cstdlib>
static void require(bool ok,int line){if(!ok){std::fprintf(stderr,"Results controller check failed at %d\n",line);std::exit(1);}}
#define require(ok) require((ok),__LINE__)
int main(int argc,char** argv){
    QApplication app(argc,argv);LiveMenuSession session("/unused");QuickBattleResultWidget widget;
    LiveResultMenuController controller(session,widget);MenuBridge::State state;
    state.screen=MNM_MENU_RESULT_SCREEN;state.ready=1;state.results.context=1;state.results.actions=3;
    state.results.players[0]={true,"<Player>","12","3","10","132"};
    require(controller.present(state));widget.show();app.processEvents();
    auto* quit=widget.findChild<QPushButton*>("quickResultAction2");quit->setFocus();
    state.results.players[0].kills="13";++state.generation;require(controller.present(state));app.processEvents();
    require(quit->hasFocus()&&widget.findChild<QLabel*>("quickResultText11")->text()=="13");
    require(widget.findChild<QLabel*>("quickResultText7")->text()=="<Player>"&&widget.findChild<QLabel*>("quickResultText7")->textFormat()==Qt::PlainText);
    state.results.players[0].active=false;state.results.actions=1;require(controller.present(state));app.processEvents();
    require(widget.findChild<QLabel*>("quickResultText7")->isHidden()&&widget.findChild<QPushButton*>("quickResultAction1")->hasFocus()&&!quit->isEnabled());
    state.results.context=2;QString error;require(!controller.present(state,&error)&&!error.isEmpty());
    std::puts("Results controller checks passed");
}
