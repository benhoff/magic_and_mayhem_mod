#include "live_result_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v5.h"
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QTemporaryDir>
#include <QtEndian>
#include <QKeyEvent>
#include <cstdio>
#include <cstdlib>
static void require(bool ok,int line){if(!ok){std::fprintf(stderr,"Results controller check failed at %d\n",line);std::exit(1);}}
#define require(ok) require((ok),__LINE__)
struct LiveMenuSessionTestAccess {
    static void initialize(LiveMenuSession& s,const QString& path){
        s.bridge_=std::make_unique<MenuBridge>();require(s.bridge_->create(path,true,true,false,true));s.active_=true;s.clock_.start();
    }
    static void poll(LiveMenuSession& s){s.poll();}
    static void deactivate(LiveMenuSession& s){s.active_=false;}
    static void advanceMedia(LiveMenuSession& s){s.lastState_-=10001;s.requestedAt_-=10001;s.resultsRequestedAt_-=10001;s.mediaStartedAt_-=10001;}
    static void expire(LiveMenuSession& s){s.resultsRequestedAt_=-10001;}
};
static void timingChecks(){
    QTemporaryDir root;require(root.isValid());
    for(unsigned variant=0;variant<14;++variant){
        LiveMenuSession session("/unused");QuickBattleResultWidget widget;LiveResultMenuController controller(session,widget);
        const auto path=root.filePath(QString("results-%1.bin").arg(variant));LiveMenuSessionTestAccess::initialize(session,path);
        QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V5_SIZE);require(bytes);
        auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes+word*4);};
        auto get=[&](int word){return qFromLittleEndian<quint32>(bytes+word*4);};
        quint32 seq=0;unsigned handoffs=0,failures=0;bool enabled=false;
        session.stateChanged=[&](const MenuBridge::State& state){require(controller.present(state));widget.setEnabled(state.ready);enabled=state.ready;};
        session.failed=[&](const QString&){++failures;};session.originalViewportRequested=[&]{++handoffs;};
        auto state=[&](quint32 generation,quint32 ack,quint32 ready,quint32 status=MNM_MENU_OK,quint32 thread=123,quint32 screen=26,quint32 depth=1,quint32 actions=3){
            put(32,++seq);put(33,generation);put(34,screen);put(35,ready);put(36,ack);put(37,status);put(38,thread);put(39,0);
            constexpr int m=MNM_MENU_V5_RESULTS/4;put(m,actions);put(m+1,1);put(m+2,depth);put(m+3,0);
            for(int i=0;i<4;++i){put(m+4+i*162,0);put(m+5+i*162,UINT32_MAX);}put(32,++seq);
        };
        state(1,0,1);LiveMenuSessionTestAccess::poll(session);require(enabled);
        // The engine changes between presentation and the click. No stale
        // display generation is used, and temporarily unready input is retained.
        state(2,0,variant==0?0:1);
        auto* quit=widget.findChild<QPushButton*>("quickResultAction2");require(quit);quit->click();
        require(!enabled&&session.requestResults(MNM_MENU_RESULT_QUIT)&&!session.requestResults(MNM_MENU_RESULT_CONTINUE));
        require(get(7)==(variant==0?0u:1u));
        if(variant==0){LiveMenuSessionTestAccess::poll(session);require(get(7)==0);state(3,0,1);LiveMenuSessionTestAccess::poll(session);require(get(7)==1);}
        if(variant==1){
            // Held Escape repeats and rapid duplicate clicks cannot create a
            // second request while an acknowledgement is outstanding.
            for(int i=0;i<8;++i){QKeyEvent repeat(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier,QString(),true);QApplication::sendEvent(&widget,&repeat);quit->click();LiveMenuSessionTestAccess::poll(session);}
            require(get(7)==1);
        }
        if(variant>=2&&variant<=8){
            state(4,variant==4?2:1,0,variant==2?MNM_MENU_UNAVAILABLE:variant==8?MNM_MENU_UNSUPPORTED:MNM_MENU_STALE,
                  variant==5?124:123,variant==6?0:26,variant==7?2:1);
            LiveMenuSessionTestAccess::poll(session);
            if(variant<=3){
                require(!failures&&!handoffs&&get(7)==1&&!enabled);
                LiveMenuSessionTestAccess::poll(session);require(get(7)==1); // Still unready.
                state(5,1,1);LiveMenuSessionTestAccess::poll(session);require(get(7)==2&&!enabled);
            }else {require(failures==1&&!handoffs&&get(7)==1);peer.unmap(bytes);continue;}
        }
        if(variant==13){session.setMediaActive(true);LiveMenuSessionTestAccess::advanceMedia(session);LiveMenuSessionTestAccess::poll(session);require(!failures&&get(7)==1&&!session.requestResults(MNM_MENU_RESULT_QUIT));session.setMediaActive(false);LiveMenuSessionTestAccess::poll(session);require(!failures&&get(7)==1);}
        if(variant==12){LiveMenuSessionTestAccess::deactivate(session);require(!session.requestResults(MNM_MENU_RESULT_QUIT));require(get(7)==1&&!handoffs);peer.unmap(bytes);continue;}
        if(variant==9){LiveMenuSessionTestAccess::expire(session);LiveMenuSessionTestAccess::poll(session);require(failures==1&&!handoffs&&get(7)==1);peer.unmap(bytes);continue;}
        if(variant==10){state(4,1,1,MNM_MENU_RETIRED);LiveMenuSessionTestAccess::poll(session);require(failures==1&&!handoffs&&get(7)==1);peer.unmap(bytes);continue;}
        if(variant==11){state(4,1,0,MNM_MENU_STALE,123,26,1,1);LiveMenuSessionTestAccess::poll(session);require(failures==1&&!handoffs&&get(7)==1);peer.unmap(bytes);continue;}
        const auto request=get(7);state(6,request,0);LiveMenuSessionTestAccess::poll(session);require(!failures&&handoffs==1);
        for(int i=0;i<4;++i){state(7+i,request,0);LiveMenuSessionTestAccess::poll(session);require(!session.requestResults(MNM_MENU_RESULT_QUIT));require(handoffs==1&&get(7)==request);}
        peer.unmap(bytes);
    }
}
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
    timingChecks();
    std::puts("Results controller checks passed: 14 timing/identity cases");
}
