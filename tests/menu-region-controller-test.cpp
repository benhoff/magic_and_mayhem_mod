#include "live_region_entry_controller.hpp"
#include <QApplication>
#include <QPushButton>
#include <QRadioButton>
#include <QKeyEvent>
#include <cstdio>
#include <cstdlib>
#define require(ok) do{if(!(ok)){std::fprintf(stderr,"Region controller failed at %d\n",__LINE__);std::exit(1);}}while(0)
int main(int argc,char** argv){
 QApplication app(argc,argv);LiveMenuSession session("/unused");RegionEntryWidget widget;LiveRegionEntryController controller(session,widget);
 MenuBridge::State s;s.screen=18;s.ready=1;s.region={4,4,0,2,15,1,1,"Celtic","Forest of Pain"};int changed=0,cancelled=0;
 QObject::connect(&widget,&RegionEntryWidget::difficultyChanged,[&](auto){++changed;});QObject::connect(&widget,&RegionEntryWidget::cancelled,[&]{++cancelled;});
 require(controller.present(s)&&changed==0&&widget.region().name=="Forest of Pain"&&int(widget.region().difficulty)==2);
 require(!widget.findChild<QPushButton*>("regionEntryEnter")->isEnabled());for(int i=0;i<3;++i)require(!widget.findChild<QPushButton*>(QString("regionEntryAuxiliary%1").arg(i))->isEnabled());
 auto* radio=widget.findChild<QRadioButton*>("regionEntryDifficulty3");radio->click();require(changed==1&&int(widget.region().difficulty)==3);
 s.region.difficulty=1;require(controller.present(s)&&int(widget.region().difficulty)==3); // Unsent draft survives engine projection.
 s.region.available=3;s.region.actions=0;require(controller.present(s)&&!radio->isEnabled());QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(&widget,&escape);require(cancelled==0);
 require(!widget.setDifficulty(RegionEntryWidget::Difficulty(4))&&int(widget.region().difficulty)==3);
 s.screen=3;require(controller.present(s));s.screen=18;s.region.difficulty=1;require(controller.present(s)&&int(widget.region().difficulty)==1&&changed==1);
 s.screen=3;require(controller.present(s));s.screen=18;s.region.caller=22;QString error;require(!controller.present(s,&error)&&!error.isEmpty());
 std::puts("Region controller checks passed");
}
