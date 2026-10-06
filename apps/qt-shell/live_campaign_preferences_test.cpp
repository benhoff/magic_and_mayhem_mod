#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "mini_menu_widget.hpp"
#include "preferences_widget.hpp"
#include "battle_result_widget.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QProcess>
#include <QTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QtEndian>
#include <memory>
void installLiveCampaignPreferencesTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
 session.campaignPreferencesEnabled=true;
 installLiveRegionEnterTest(app,window,session,path);
 session.smokeSeconds=0;
 struct Evidence {int menus=0,prefs=0,returns=0,resumes=0;bool game=false,waiting=false,closing=false,quit=false,main=false,defeat=false,cancelUnchanged=false,storeVerified=false;quint32 thread=0;std::array<int,7> initial{},committed{};QByteArray beforeCancel,acceptedStore;QJsonArray states;QString error;};auto e=std::make_shared<Evidence>();
 auto failed=session.failed;auto fail=[e,failed](const QString& error){e->error=error;if(failed)failed(error);};
 auto* input=new QTimer(&window);input->setInterval(1000);
 QObject::connect(input,&QTimer::timeout,&window,[e,fail,&session]{
  if(!e->game||e->waiting||e->menus>=4)return;
  QFile events(QDir(session.evidenceDirectory()).filePath("events.bin"));if(!events.open(QIODevice::ReadOnly))return;
  const auto bytes=events.readAll();int ticks=0,resumes=0;
  for(int at=16;at+64<=bytes.size();at+=64){auto word=[&](int i){return qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(bytes.constData()+at+4*i));};if(word(1)==12&&word(3)==2&&word(6)==1&&word(11)==0x6cbb78)++ticks;if(word(1)==20&&word(11)==0x6cbb78)++resumes;}
  if(ticks<3||resumes<e->resumes)return;
  if(QProcess::execute("python3",{QDir::current().filePath("tools/test-live-campaign-mini.py"),"--send-escape"})!=0)fail("Original Escape injection failed.");
 });input->start();
 auto battle=session.battleStarted;session.battleStarted=[e,battle](quint32 d){if(battle)battle(d);e->game=d==3;};
 auto viewport=session.originalViewportRequested;session.originalViewportRequested=[e,viewport,fail,&session,&window,path]{
  if(viewport){viewport();}
  ++e->returns;
  if(e->closing){e->closing=false;++e->resumes;e->waiting=false;return;}
  if(!e->quit||e->defeat)return;
  auto* container=window.findChild<QWidget*>("legacyGameContainer");if(!container||container->width()<800||container->height()<600){fail("Original confirmation viewport unavailable.");return;}
  auto origin=container->mapToGlobal(QPoint((container->width()-800)/2,(container->height()-600)/2));auto* answer=new QProcess(&window);answer->setProcessChannelMode(QProcess::MergedChannels);
  QObject::connect(answer,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[answer,fail](int code,QProcess::ExitStatus status){if(code||status!=QProcess::NormalExit)fail("Original confirmation input failed: "+QString::fromLocal8Bit(answer->readAll()));answer->deleteLater();});
  answer->start("python3",{QDir::current().filePath("tools/test-live-qt-campaign-defeat.py"),"--origin-x",QString::number(origin.x()),"--origin-y",QString::number(origin.y()),"--answer","yes","--experiment",session.evidenceDirectory(),"--capture",QFileInfo(path).dir().filePath("original-confirmation.png")});
 };
 auto state=session.stateChanged;session.stateChanged=[e,state,fail,&session,&window,path](const MenuBridge::State& s){
  if(state){state(s);}
  if(!s.ready)return;
  if(s.screen==17&&!e->waiting){
   const quint32 acks[]={3,6,9,11};if(e->menus>=4||s.ack!=acks[e->menus]||s.mini.context!=5||s.mini.mode!=2||s.mini.depth!=5||s.mini.actions!=7){fail("Unexpected campaign Mini ownership/ack.");return;}
   auto* w=window.findChild<MiniMenuWidget*>();if(!w||!w->isVisible()){fail("Native campaign Mini unavailable.");return;}
   for(int i=1;i<=5;++i){auto* b=w->findChild<QPushButton*>(QString("miniMenuButton%1").arg(i));if(!b||b->isEnabled()!=(i>=3)){fail("Campaign Mini control availability mismatch.");return;}}
   e->thread=s.thread;e->states.append(QJsonObject{{"screen",17},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)}});e->waiting=true;e->quit=++e->menus==4;
   QTimer::singleShot(0,&window,[e,w,&window,path]{window.grab().save(QFileInfo(path).dir().filePath(QString("qt-mini-%1.png").arg(e->menus)));w->findChild<QPushButton*>(e->quit?"miniMenuButton4":"miniMenuButton3")->click();});return;
  }
  if(s.screen==10){
   const quint32 acks[]={4,5,7,8,10};if(e->prefs>=5||s.ack!=acks[e->prefs])return;
   if(s.thread!=e->thread||s.preferences.parentScreen!=17||s.preferences.depth!=6){fail("Campaign Preferences caller/thread mismatch.");return;}
   auto* w=window.findChild<PreferencesWidget*>();if(!w||!w->isVisible()){fail("Native campaign Preferences unavailable.");return;}
   QJsonArray values;for(int v:s.preferences.values)values.append(v);e->states.append(QJsonObject{{"screen",10},{"ack",int(s.ack)},{"generation",int(s.generation)},{"thread",int(s.thread)},{"parent",int(s.preferences.parentScreen)},{"depth",int(s.preferences.depth)},{"available",int(s.preferences.available)},{"values",values}});int stage=e->prefs++;
   QTimer::singleShot(0,&window,[e,w,stage,s,fail,&session,&window,path]{
    window.grab().save(QFileInfo(path).dir().filePath(QString("qt-preferences-%1.png").arg(stage)));
    auto read=[&](const QString& file){QFile f(file);if(!f.open(QIODevice::ReadOnly)){fail("Cannot read preference evidence: "+file);return QByteArray{};}return f.readAll();};const auto profile=QDir(session.evidenceDirectory()).filePath("game/CFG/prefs.cfg");
    if(stage==0||stage==2){
     if(stage==0){e->initial=s.preferences.values;e->beforeCancel=read(profile);e->committed=e->initial;e->committed[1]=e->initial[1]==-500?-1000:-500;e->committed[4]=(e->initial[4]+1)%3;}
     else{e->cancelUnchanged=read(profile)==e->beforeCancel;if(!e->cancelUnchanged||s.preferences.values!=e->initial){fail("Cancel did not restore engine values/file.");return;}}
     auto* radio=w->findChild<QRadioButton*>(QString("preferencesRadio%1").arg(5+e->committed[4]));auto* fx=w->findChild<QSlider*>("preferencesSlider2");if(!radio||!radio->isEnabled()||!fx||!fx->isEnabled()){fail("Selected preference controls unavailable.");return;}radio->click();fx->setValue(e->committed[1]);return;
    }
    if(stage==1||stage==3){if(s.preferences.values[1]!=e->committed[1]||s.preferences.values[4]!=e->initial[4]||int(w->draftSettings().dialogueSpeed)!=e->committed[4]){fail("Original preview or native radio draft mismatch.");return;}e->closing=true;w->findChild<QPushButton*>(stage==1?"preferencesCancel":"preferencesOk")->click();return;}
    std::array<int,7> actual{};const auto store=QDir(QFileInfo(path).absolutePath()).filePath("config/mnm-qt-shell/engine-preferences.json");e->acceptedStore=read(store);auto json=QJsonDocument::fromJson(e->acceptedStore).object()["values"].toArray();for(int i=0;i<7;++i)actual[i]=json[i].toInt();
    e->storeVerified=actual==e->committed;if(!e->storeVerified||s.preferences.values!=e->committed||!EnginePreferencesStore::readProfile(profile,actual)||actual!=e->committed){fail("Original profile, persisted store and reopened settings disagree.");return;}e->closing=true;w->findChild<QPushButton*>("preferencesCancel")->click();
   });return;
  }
  if(e->quit&&s.screen==6&&!e->defeat){if(s.ack!=12||s.thread!=e->thread||s.defeat.texts[19]!="You quit the battle."){fail("Original defeat report mismatch.");return;}e->defeat=true;e->states.append(QJsonObject{{"screen",6},{"ack",12},{"generation",int(s.generation)},{"thread",int(s.thread)}});QTimer::singleShot(0,&window,[&window]{window.findChild<QPushButton*>("battleResultContinue")->click();});return;}
  if(e->defeat&&s.screen==3&&!e->main){if(s.ack!=13||s.handoff||s.thread!=e->thread){fail("Fresh Main ownership mismatch.");return;}e->main=true;e->game=false;QTimer::singleShot(0,&window,[&window]{window.findChild<QPushButton*>("mainMenuAction4")->click();});}
 };
 auto finished=session.finished;session.finished=[e,finished,path,&app]{
  if(finished){finished();}
  QFile f(path);if(!f.open(QIODevice::ReadOnly)){app.exit(1);return;}auto r=QJsonDocument::fromJson(f.readAll()).object();f.close();QFile store(QFileInfo(path).dir().filePath("config/mnm-qt-shell/engine-preferences.json"));bool unchanged=store.open(QIODevice::ReadOnly)&&store.readAll()==e->acceptedStore;
  bool ok=r["success"].toBool()&&e->menus==4&&e->prefs==5&&e->returns==8&&e->resumes==3&&e->cancelUnchanged&&e->storeVerified&&unchanged&&e->defeat&&e->main&&e->error.isEmpty();QJsonArray initial,committed;for(int v:e->initial)initial.append(v);for(int v:e->committed)committed.append(v);
  r["success"]=ok;r["campaign_preferences_states"]=e->states;r["viewport_returns"]=e->returns;r["initial"]=initial;r["committed"]=committed;r["cancel_file_unchanged"]=e->cancelUnchanged;r["store_verified"]=e->storeVerified;r["final_cancel_store_unchanged"]=unchanged;r["main_return"]=e->main;r["preferences_error"]=e->error;r["scope"]="Fresh campaign original Escape, native Mini/Preferences preview/Cancel rollback and World resume, repeat preview/OK persistence and World resume, reopen saved settings/Cancel, native Quit/original Yes/native defeat OK/Main Quit; resolution/game-speed changes, pause timing and other callers pending";
  if(!f.open(QIODevice::WriteOnly)||f.write(QJsonDocument(r).toJson())<0||!f.flush()){ok=false;}
  app.exit(ok?0:1);
 };
}
