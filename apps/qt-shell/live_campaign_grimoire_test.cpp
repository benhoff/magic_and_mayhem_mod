#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "grimoire_widget.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
void installLiveCampaignGrimoireTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    struct Evidence{int stage=0;quint32 thread=0;QString error;QJsonArray states;};auto e=std::make_shared<Evidence>();
    auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"error",e->error},{"scope","V22 fresh Celtic first Region,original graphics/audio,native static book contents/Intro2/Celtic contents/Forest and original Close/Region Cancel/Main Quit. No victory,loaded Realm,other realms,dynamic values,pagination,research or persistence."}}).toJson())>0&&f.flush();};
    auto fail=[e,save,&app](const QString& why){if(!e->error.isEmpty())return;e->error=why;std::fprintf(stderr,"Campaign Grimoire: %s\n",qPrintable(why));save(false);app.exit(1);};
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};auto failure=session.failed;session.failed=[failure,fail](const QString& why){if(failure)failure(why);fail(why);};
    auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
        if(presentation)presentation(s);
        if(!e->error.isEmpty())return;
        const quint32 screens[]={3,18,27,27,27,27,27,18,3},chapters[]={0,0,1,0,0,1,1,0,0},sections[]={0,0,1,0,2,0,1,0,0};
        if(!s.ready||e->stage>=9||s.screen!=screens[e->stage]||s.ack!=quint32(e->stage))return;
        const int stage=e->stage;if((stage&&s.status!=MNM_MENU_OK)||(e->thread&&s.thread!=e->thread)||(s.screen==27&&(!s.grimoire.broader||s.grimoire.chapter!=chapters[stage]||s.grimoire.section!=sections[stage]))){fail("Original location/thread/status differs.");return;}
        e->thread=s.thread;e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"status",int(s.status)},{"thread",int(s.thread)},{"chapter",int(s.grimoire.chapter)},{"section",int(s.grimoire.section)},{"level",int(s.grimoire.level)},{"wizard",int(s.grimoire.wizard)},{"entries",s.grimoire.entries.size()}});++e->stage;
        QTimer::singleShot(0,&window,[stage,fail,&window,path]{
            window.grab().save(QFileInfo(path).dir().filePath(QString("campaign-book-%1.png").arg(stage)));
            auto click=[&](const char* name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isEnabled()||!b->isVisible()){fail(QString("Unavailable native control: ")+name);return;}b->click();};
            if(stage==0){click("mainMenuAction0");return;}if(stage==1){click("regionEntryAuxiliary0");return;}if(stage==7){click("regionEntryCancel");return;}if(stage==8){click("mainMenuAction4");return;}
            if(stage==2){click("grimoireTab0");return;}if(stage==4){click("grimoireTab1");return;}if(stage==6){click("grimoireClose");return;}
            auto* list=window.findChild<QListWidget*>("grimoireEntries");if(!list||!list->isVisible()){fail("Native contents missing.");return;}const int section=stage==3?2:1;
            for(int i=0;i<list->count();++i)if(list->item(i)->data(Qt::UserRole).toInt()==section){list->itemActivated(list->item(i));return;}
            fail("Original static destination absent from native contents.");
        });
    };
    auto finished=session.finished;session.finished=[e,save,finished,&app]{const bool ok=e->stage==9&&e->error.isEmpty();const bool saved=save(ok);if(finished)finished();app.exit(ok&&saved?0:1);};
    auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Bounded campaign Grimoire run expired.");});};session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
    QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
