#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "grimoire_widget.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QTextBrowser>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <memory>
#include <cstdio>
static void installCampaignCatalogTest(QApplication&,QMainWindow&,LiveMenuSession&,const QString&);
void installLiveCampaignGrimoireTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    if(!qEnvironmentVariableIsEmpty("MNM_LIVE_MENU_TEST_CAMPAIGN_CATALOG")){installCampaignCatalogTest(app,window,session,path);return;}
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
static void installCampaignCatalogTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    struct Visit{int chapter,section,level;};
    struct Evidence{int stage=0,next=0;quint32 thread=0;QString error;QJsonArray states,catalog;QVector<Visit> visits;QMap<int,int> entries;};
    auto e=std::make_shared<Evidence>();
    auto save=[e,path](bool success){QFile f(path);return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"catalog",e->catalog},{"error",e->error},{"scope","Genuine fresh Celtic first Region,V22 native static reader: every engine-admitted catalog entry through visible native tabs/contents,original selector/Close/Region Cancel/Main Quit. Original graphics/audio. No loaded campaign,progress grants,research,dynamic fields,multipage text or visual equivalence."}}).toJson())>0&&f.flush();};
    auto fail=[e,save,&app](const QString& why){if(!e->error.isEmpty())return;e->error=why;std::fprintf(stderr,"Campaign catalog: %s\n",qPrintable(why));save(false);app.exit(1);};
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failure=session.failed;session.failed=[failure,fail](const QString& why){if(failure)failure(why);fail(why);};
    auto presentation=session.stateChanged;session.stateChanged=[e,presentation,fail,&window,path](const MenuBridge::State& s){
        if(presentation)presentation(s);
        if(!e->error.isEmpty()||!s.ready||s.ack!=quint32(e->stage))return;
        const int stage=e->stage;const int count=e->visits.size();
        const quint32 expected=stage==0||stage==count+4?3:stage==1||stage==count+3?18:27;
        if(s.screen!=expected)return;
        if((stage&&s.status!=MNM_MENU_OK)||(e->thread&&s.thread!=e->thread)){fail("Original status/thread differs.");return;}
        auto* widget=window.findChild<GrimoireWidget*>();
        if(s.screen==27){
            if(!s.grimoire.broader||!widget){fail("Native campaign reader missing.");return;}
            if(stage==2){
                e->entries=s.grimoire.entries;
                for(auto it=e->entries.cbegin();it!=e->entries.cend();++it){const int chapter=int(it.key()/124),section=int(it.key()%124),level=int(it.value());
                    if(chapter>=widget->book().chapters.size()||level!=0){fail("Unexpected fresh campaign catalog.");return;}
                    e->catalog.append(QJsonObject{{"chapter",chapter},{"section",section},{"level",level}});
                    if(section)e->visits.append({chapter,0,0});
                    e->visits.append({chapter,section,level});
                }
                if(e->catalog.size()!=22){fail("Fresh original catalog changed; review before widening this scenario.");return;}
            }else if(s.grimoire.entries!=e->entries){fail("Engine availability changed during navigation.");return;}
            const Visit target=stage==2?Visit{1,1,0}:e->visits[stage-3];
            const auto location=widget->location();
            if(s.grimoire.chapter!=quint32(target.chapter)||s.grimoire.section!=quint32(target.section)||s.grimoire.level!=quint32(target.level)||location.chapter!=target.chapter||location.section!=target.section||location.level!=target.level||location.page){fail("Native location differs from original ACK.");return;}
            if(target.section){auto* text=window.findChild<QTextBrowser*>("grimoireText");if(!text||!text->isVisible()||text->toPlainText().trimmed().isEmpty()){fail("Native static article is empty or hidden.");return;}}
        }
        e->thread=s.thread;e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"status",int(s.status)},{"thread",int(s.thread)},{"chapter",int(s.grimoire.chapter)},{"section",int(s.grimoire.section)},{"level",int(s.grimoire.level)},{"wizard",int(s.grimoire.wizard)},{"entries",s.grimoire.entries.size()}});++e->stage;
        QTimer::singleShot(0,&window,[e,stage,fail,&window,path]{
            if(!window.grab().save(QFileInfo(path).dir().filePath(QString("campaign-catalog-%1.png").arg(stage)))){fail("Screenshot could not be retained.");return;}
            auto click=[&](const QString& name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isEnabled()||!b->isVisible()){fail("Unavailable native control: "+name);return;}b->click();};
            if(stage==0){click("mainMenuAction0");return;}if(stage==1){click("regionEntryAuxiliary0");return;}
            if(stage==e->visits.size()+3){click("regionEntryCancel");return;}if(stage==e->visits.size()+4){click("mainMenuAction4");return;}
            if(e->next==e->visits.size()){click("grimoireClose");return;}
            const Visit visit=e->visits[e->next++];
            if(!visit.section){click(QString("grimoireTab%1").arg(visit.chapter));return;}
            auto* list=window.findChild<QListWidget*>("grimoireEntries");if(!list||!list->isVisible()){fail("Native contents missing.");return;}
            for(int i=0;i<list->count();++i)if(list->item(i)->data(Qt::UserRole).toInt()==visit.section){list->itemActivated(list->item(i));return;}
            fail("Engine-admitted entry absent from native contents.");
        });
    };
    auto finished=session.finished;session.finished=[e,save,finished,&app]{const bool ok=e->stage==e->visits.size()+5&&e->catalog.size()==22&&e->error.isEmpty();const bool saved=save(ok);if(finished)finished();app.exit(ok&&saved?0:1);};
    auto launched=session.launched;session.launched=[launched,fail,&window]{if(launched)launched();QTimer::singleShot(180000,&window,[fail]{fail("Bounded campaign catalog run expired.");});};
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
    QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
