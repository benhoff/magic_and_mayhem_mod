#include "menu_bridge.hpp"
#include "live_campaign_defeat_controller.hpp"
#include "../protocols/include/mnm/menu_v11.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QLabel>
#include <QPushButton>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok){if(!ok)std::abort();}
int main(int argc,char** argv){
 QApplication app(argc,argv);QTemporaryDir dir;require(dir.isValid());MenuBridge bridge;
 require(bridge.create(dir.filePath("channel"),true,true,true,true,true,true,true,true,true,true));
 QFile peer(dir.filePath("channel"));require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V11_SIZE);require(bytes);
 auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes+4*word);};quint32 sequence=0;constexpr int m=MNM_MENU_V11_DEFEAT/4;
 auto state=[&](quint32 gen,quint32 ack,quint32 ready){
  std::memset(bytes+MNM_MENU_V11_DEFEAT,0,MNM_MENU_V11_DEFEAT_SIZE);
  put(32,++sequence);put(33,gen);put(34,6);put(35,ready);put(36,ack);put(37,0);put(38,123);put(39,0);put(m,1);put(m+1,5);put(m+2,5);put(m+3,0);
  std::strcpy(reinterpret_cast<char*>(bytes)+MNM_MENU_V11_DEFEAT+16,"Defeat! (Forest)");
  for(int i=0;i<21;++i)std::strcpy(reinterpret_cast<char*>(bytes)+MNM_MENU_V11_DEFEAT+272+i*256,i==19?"You quit the battle.":"Original display");
  put(32,++sequence);
 };
 MenuBridge::State s;state(1,0,1);require(bridge.read(s)&&s.defeat.title=="Defeat! (Forest)"&&s.defeat.texts[19]=="You quit the battle.");
 LiveMenuSession session(dir.path());BattleResultWidget widget;LiveCampaignDefeatController controller(session,widget);require(controller.present(s));
 require(widget.findChild<QLabel*>("battleResultTitle")->text()==s.defeat.title);
 for(int i=0;i<21;++i)require(widget.findChild<QLabel*>(QString("battleResultText%1").arg(i+1))->text()==s.defeat.texts[i]);
 auto unsupported=s;unsupported.defeat.outcome=1;require(!controller.present(unsupported));session.campaignDefeatEnabled=false;require(!controller.present(s));session.campaignDefeatEnabled=true;
 require(!bridge.request(MNM_MENU_RESULT_QUIT,s)&&bridge.request(MNM_MENU_DEFEAT_OK,s)&&!bridge.request(MNM_MENU_DEFEAT_OK,s));
 state(2,1,1);require(!bridge.request(MNM_MENU_DEFEAT_OK,s));require(bridge.read(s));state(3,1,0);require(bridge.read(s)&&!bridge.request(MNM_MENU_DEFEAT_OK,s));
 for(int field:{m,m+1,m+2,m+3}){state(4,1,1);put(field,99);require(!bridge.read(s));}
 state(5,1,1);bytes[MNM_MENU_V11_DEFEAT+16]=0;require(!bridge.read(s));
 for(int i=0;i<22;++i){state(6,1,1);std::memset(bytes+MNM_MENU_V11_DEFEAT+16+256*i,'x',256);require(!bridge.read(s));}
 state(7,1,1);bytes[MNM_MENU_V11_DEFEAT+16]=0x80;require(bridge.read(s)&&s.defeat.title[0]==QChar(0x20ac));
 state(8,1,1);put(32,sequence+1);require(!bridge.read(s));state(9,1,1);require(bridge.read(s));bridge.retire();require(!bridge.request(MNM_MENU_DEFEAT_OK,s));
 peer.unmap(bytes);std::puts("Campaign defeat bridge/controller checks passed");
}
