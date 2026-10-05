#include "menu_bridge.hpp"
#include "live_spell_menu_controller.hpp"
#include "../../protocols/include/mnm/menu_v3.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QPushButton>
#include <QtEndian>
#include <cstring>
#include <cassert>
int main(int argc,char** argv){
    QApplication app(argc,argv);QTemporaryDir dir;MenuBridge bridge;const auto path=dir.filePath("v3.bin");assert(bridge.create(path,true,true));
    QFile f(path);assert(f.open(QIODevice::ReadWrite));auto* bytes=f.map(0,MNM_MENU_V3_SIZE);assert(bytes);
    auto put=[&](int offset,quint32 value){qToLittleEndian(value,bytes+offset);};auto text=[&](int offset,const char* value){std::strcpy(reinterpret_cast<char*>(bytes)+offset,value);};
    put(128,2);put(132,10);put(136,7);put(140,1);put(148,MNM_MENU_OK);put(152,42);put(156,1);
    const int base=MNM_MENU_V3_SPELL;put(base,0);put(base+4,30);put(base+8,3);
    for(int a=0;a<3;++a){put(base+MNM_MENU_V3_COUNTS+4*a,2);for(int i=0;i<21;++i)put(base+MNM_MENU_V3_SLOTS+4*(a*21+i),UINT32_MAX);}
    for(int i=0;i<25;++i)put(base+MNM_MENU_V3_SHELVES+4*i,i<2?quint32(i):UINT32_MAX);
    for(int i=0;i<2;++i){text(base+MNM_MENU_V3_ITEM_NAMES+i*128,i?"Second":"First");for(int a=0;a<3;++a){put(base+MNM_MENU_V3_RECIPES+4*(i*3+a),i*3+a);text(base+MNM_MENU_V3_SPELL_NAMES+(i*3+a)*128,"Recovered spell");}}
    MenuBridge::State state;assert(bridge.read(state)&&state.spells.offered==3&&state.spells.counts[0]==2);
    put(base+MNM_MENU_V3_COUNTS,8);assert(!bridge.read(state));put(base+MNM_MENU_V3_COUNTS,2);
    put(base+MNM_MENU_V3_SHELVES+4,0);assert(!bridge.read(state));put(base+MNM_MENU_V3_SHELVES+4,1);
    std::array<int,63> assignments;assignments.fill(-1);assignments[0]=0;assignments[21]=0;assert(!bridge.finishSpells(state,assignments));
    assignments[21]=2;assert(!bridge.finishSpells(state,assignments));assignments[21]=1;
    assert(bridge.finishSpells(state,assignments));bridge.heartbeat();assert(qFromLittleEndian<quint32>(bytes+MNM_MENU_V3_HOST_SLOTS)==0&&qFromLittleEndian<quint32>(bytes+MNM_MENU_V3_HOST_SLOTS+21*4)==1);
    assert(!bridge.finishSpells(state,assignments));
    // Presentation uses engine IDs/recipes, and timer-only refresh preserves draft.
    LiveMenuSession session(dir.path());SpellboxWidget widget;LiveSpellMenuController controller(session,widget);QString error;
    assert(controller.present(state,&error));assert(widget.assignItem("0","0",&error));
    auto request=widget.draftRequest();assert(request.assignments[0].spellId=="0");state.generation++;state.spells.seconds--;
    assert(controller.present(state,&error)&&widget.draftRequest().assignments[0].itemId=="0");
    widget.findChild<QPushButton*>("spellboxCancel")->click();assert(widget.draftRequest().assignments[0].itemId.isEmpty());
    bridge.retire();assert(!bridge.finishSpells(state,assignments));f.unmap(bytes);return 0;
}
