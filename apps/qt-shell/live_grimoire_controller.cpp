#include "live_grimoire_controller.hpp"
#include "../../protocols/include/mnm/menu_v14.h"
LiveGrimoireController::LiveGrimoireController(LiveMenuSession& session,GrimoireWidget& widget):session_(session),widget_(widget){
    widget_.setLiveCampaignReader(true);
    QObject::connect(&widget_,&GrimoireWidget::locationRequested,&widget_,[this](const auto& where){if(entered_&&!pending_){action_=MNM_MENU_GRIMOIRE_SELECT;section_=session_.campaignBookEnabled?quint32(where.chapter*124+where.section):where.chapter==1?5:quint32(where.section);pending_=true;pump();}});
    QObject::connect(&widget_,&GrimoireWidget::closed,&widget_,[this]{if(entered_&&!pending_){action_=MNM_MENU_GRIMOIRE_CLOSE;section_=0;pending_=true;pump();}});
    timer_.setInterval(50);QObject::connect(&timer_,&QTimer::timeout,&widget_,[this]{pump();});timer_.start();
}
void LiveGrimoireController::pump(){if(pending_&&session_.request(action_,section_)){pending_=false;entered_=false;}}
bool LiveGrimoireController::present(const MenuBridge::State& state,QString* error){
    if(state.screen!=MNM_MENU_GRIMOIRE_SCREEN){entered_=pending_=false;return true;}
    if(!state.ready)return true;
    const auto& b=state.grimoire;
    if(b.broader){
        if(!session_.campaignBookEnabled){if(error)*error="Broader Grimoire is unavailable.";return false;}
        QMap<int,int> admitted;const auto& book=widget_.book();
        for(auto it=b.entries.cbegin();it!=b.entries.cend();++it){const int chapter=it.key()/124,section=it.key()%124;if(chapter>=book.chapters.size())continue;if(!section){admitted.insert(it.key(),0);continue;}
            for(const auto& entry:book.chapters[chapter].entries)if(entry.section==section&&entry.levels.contains(it.value())){const auto& text=entry.levels[it.value()];if(text.pages.size()==1&&text.dynamicLabels.isEmpty())admitted.insert(it.key(),it.value());}}
        const int selected=int(b.chapter*124+b.section);if(!admitted.contains(selected)){if(error)*error="This original Grimoire page requires original controls.";return false;}
        widget_.setLiveCampaignEntries(admitted);
        const auto current=widget_.location();if((!entered_||current.chapter!=int(b.chapter)||current.section!=int(b.section)||current.level!=int(b.level))&&!widget_.setLocation({int(b.chapter),int(b.section),int(b.level),0},error))return false;
        entered_=true;return true;
    }
    widget_.setLiveCampaignEntries({},false);
    if(!((b.chapter==0&&b.section<=4)||(b.chapter==1&&b.section==1))||b.cursor!=1||b.available!=63||b.parent!=18||b.depth!=5||b.context!=5){if(error)*error="Unsupported Grimoire caller or location.";return false;}
    const auto& chapters=widget_.book().chapters;
    if(chapters.size()<2||chapters[0].entries.size()!=4){if(error)*error="Unsupported Introduction catalog.";return false;}
    for(int i=0;i<4;++i){const auto& entry=chapters[0].entries[i];if(entry.section!=i+1||entry.levels.size()!=1||!entry.levels.contains(0)||entry.levels[0].pages.size()!=1||!entry.levels[0].dynamicLabels.isEmpty()){if(error)*error="Unsupported Introduction text.";return false;}}
    bool forest=false;for(const auto& e:chapters[1].entries)if(e.section==1){forest=e.levels.size()==1&&e.levels.contains(0)&&e.levels[0].pages.size()==1&&e.levels[0].dynamicLabels.isEmpty();}
    if(!forest){if(error)*error="Unsupported current-region text.";return false;}
    const auto current=widget_.location();
    if((!entered_||current.chapter!=int(b.chapter)||current.section!=int(b.section))&&!widget_.setLocation({int(b.chapter),int(b.section),0,0},error))return false;
    entered_=true;return true;
}
