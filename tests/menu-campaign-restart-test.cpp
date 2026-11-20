#include "live_mini_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v23.h"
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(v) do{if(!(v)){std::fprintf(stderr,"Restart check failed: %d\n",__LINE__);std::exit(1);}}while(0)
struct LiveMenuSessionTestAccess {
    static MenuBridge& initialize(LiveMenuSession& s,const QString& path){
        s.bridge_=std::make_unique<MenuBridge>();
        require(s.bridge_->create(path,true,true,true,true,true,true,true,true,true,true,true,true,false,false,false,false,false,false,false,false,false,true));
        s.active_=true;s.clock_.start();return *s.bridge_;
    }
    static void poll(LiveMenuSession& s){s.poll();}
    static bool gameplay(const LiveMenuSession& s){return s.inBattle_;}
};
int main(int argc,char** argv){
    QApplication app(argc,argv);QTemporaryDir root;LiveMenuSession session("/unused");
    auto& bridge=LiveMenuSessionTestAccess::initialize(session,root.filePath("channel.bin"));
    QFile peer(root.filePath("channel.bin"));require(peer.open(QIODevice::ReadWrite));
    auto* b=peer.map(0,MNM_MENU_V23_SIZE);require(b);
    auto put=[&](int at,quint32 v){qToLittleEndian(v,b+at);};
    auto state=[&](quint32 generation,quint32 ack,quint32 actions=63,quint32 screen=17,quint32 ready=1){
        put(128,1);put(132,generation);put(136,screen);put(140,ready);put(144,ack);put(148,MNM_MENU_OK);put(152,77);put(156,3);
        const quint32 mini[]={0,0,5,2,actions,5,2,0};for(int i=0;i<8;++i)put(MNM_MENU_V4_MINI+4*i,mini[i]);put(128,2*generation);
    };
    require(qFromLittleEndian<quint32>(b+MNM_MENU_V23_BASE)==13);
    MiniMenuWidget widget;LiveMiniMenuController controller(session,widget);widget.show();
    session.stateChanged=[&](const auto& s){require(controller.present(s,nullptr));};
    state(1,0);LiveMenuSessionTestAccess::poll(session);app.processEvents();
    auto* restart=widget.findChild<QPushButton*>("miniMenuRestart");require(restart&&restart->isVisible()&&restart->isEnabled());
    restart->click();auto* dialog=widget.findChild<QMessageBox*>("restartLevelConfirmation");require(dialog&&dialog->isVisible());
    require(dialog->defaultButton()==dialog->button(QMessageBox::No)&&bridge.requestedSequence()==0);
    dialog->done(QMessageBox::No);dialog->setObjectName("cancelled");require(bridge.requestedSequence()==0);
    restart->click();dialog=widget.findChild<QMessageBox*>("restartLevelConfirmation");require(dialog&&dialog->isVisible());
    dialog->done(QMessageBox::Yes);require(bridge.requestedSequence()==1);
    require(qFromLittleEndian<quint32>(b+32)==MNM_MENU_MINI_RESTART);
    require(!session.requestMini(MNM_MENU_MINI_RESTART));
    state(2,1,31,0,0);LiveMenuSessionTestAccess::poll(session);require(LiveMenuSessionTestAccess::gameplay(session));
    state(3,1,63);LiveMenuSessionTestAccess::poll(session);require(!LiveMenuSessionTestAccess::gameplay(session));
    // A checkpoint is optional; the other five actions must remain readable.
    state(4,1,31);LiveMenuSessionTestAccess::poll(session);require(!restart->isEnabled());
    MenuBridge::State s;require(bridge.read(s)&&!bridge.request(MNM_MENU_MINI_RESTART,s));
    state(5,1,127);require(!bridge.read(s));
    state(6,1,63);require(bridge.read(s));put(MNM_MENU_V23_BASE,22);require(!bridge.read(s));put(MNM_MENU_V23_BASE,13);
    // A Yes on a dialog from an earlier menu generation cannot restart a new visit.
    LiveMenuSessionTestAccess::poll(session);restart->click();dialog=widget.findChild<QMessageBox*>("restartLevelConfirmation");
    // Previous accepted dialog is awaiting deferred deletion, so retire its name.
    if(dialog&&!dialog->isVisible()){dialog->setObjectName("accepted");dialog=widget.findChild<QMessageBox*>("restartLevelConfirmation");}
    require(dialog&&dialog->isVisible());state(7,1,63);LiveMenuSessionTestAccess::poll(session);
    dialog->done(QMessageBox::Yes);require(bridge.requestedSequence()==1);
    widget.setMode(MiniMenuWidget::Mode::Battle);require(restart->isHidden());
    peer.unmap(b);std::puts("Campaign restart wire, confirmation and ownership checks passed");
}
