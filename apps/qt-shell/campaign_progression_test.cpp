#include "campaign_smoke_test.hpp"
#include "live_menu_session.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QAbstractButton>
#include <QComboBox>
#include <QTimer>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QElapsedTimer>
#include <memory>
#include <cstdio>

// Candidate EP05 automation. Only public controls/physical World input are used.
// A completed driver is semantic evidence; independent history admission is separate.
void installCampaignProgressionTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,
        const QString& path,std::function<CampaignPresentationProbe()> probe){
    const auto mode=qEnvironmentVariable("MNM_CAMPAIGN_PROGRESSION_CASE");
    const auto size=qEnvironmentVariable("MNM_CAMPAIGN_PROGRESSION_SIZE","800x600");
    const bool low=size=="640x480";
    const int requestedDeadline=qEnvironmentVariableIntValue("MNM_CAMPAIGN_TIMEOUT_MS");
    const int deadlineMs=requestedDeadline>0?qBound(15000,requestedDeadline,900000):900000;
    bool dwellValid=true;
    const auto dwellText=qEnvironmentVariable("MNM_CAMPAIGN_STARTUP_DWELL_MS");
    const int startupDwell=dwellText.isEmpty()?0:dwellText.toInt(&dwellValid);
    struct Run {int stage=0,region=1,visits=0,input=0,realmIndex=-1,travels=0;quint32 thread=0;quint64 frames=0,paints=0,worldBaseline=0;bool done=false,helper=false,natural=false;QJsonArray states,worlds,observations;QJsonObject failure;QElapsedTimer clock;QProcess* process=nullptr;};
    auto r=std::make_shared<Run>();r->clock.start();
    auto finish=[r,path,probe,&app,mode,size,startupDwell](bool success,const QString& error){
        if(r->done)return;
        r->done=true;
        if(r->process&&r->process->state()!=QProcess::NotRunning)r->process->kill();
        const auto p=probe();QSaveFile file(path);
        const QJsonObject result{{"schema",2},{"world_observation_policy","fresh-generation-after-handoff-v1"},{"size",size},{"case",mode},{"success",success},{"error",success?QString{}:error},
            {"normal_quit",success&&r->stage==12},{"stage",r->stage},{"states",r->states},{"worlds",r->worlds},
            {"native_command_fallback",p.fallback},{"native_error",p.error},{"elapsed_ms",r->clock.elapsed()},
            {"startup_main_dwell_ms",startupDwell},
            {"pixel_history_accepted",false},{"ownership_retirement_accepted",false},{"session_observations",r->observations},{"session_failure",r->failure}};
        const auto bytes=QJsonDocument(result).toJson();
        const bool saved=file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size()&&file.commit();
        app.exit(success&&saved?0:1);
    };
    if((mode!="progression"&&mode!="victory"&&mode!="defeat")||(size!="640x480"&&size!="800x600")){
        QTimer::singleShot(0,&window,[finish]{finish(false,"Unknown progression case");});return;
    }
    if(!dwellValid||startupDwell<0||startupDwell>30000||
       (startupDwell&&qEnvironmentVariableIntValue("MNM_RESIDENT_POLL_TIMING")!=1)){
        QTimer::singleShot(0,&window,[finish]{finish(false,"Invalid diagnostic startup dwell");});return;
    }
    if(auto* scaling=window.findChild<QComboBox*>("viewportScaling"))scaling->setCurrentIndex(2);
    session.skipMoviesEnabled=qEnvironmentVariableIntValue("MNM_ACCEPTANCE_SKIP_INTROS")==1;
    auto click=[&window,finish](QString name){
        QTimer::singleShot(0,&window,[&window,finish,name]{
            auto* b=window.findChild<QAbstractButton*>(name);
            if(!b||!b->isVisible()||!b->isEnabled()){finish(false,"Public control unavailable: "+name);return;}
            b->click();
        });
    };
    auto record=[r](const MenuBridge::State& s,const QString& event){
        r->states.append(QJsonObject{{"event",event},{"screen",int(s.screen)},{"generation",int(s.generation)},
            {"sequence",int(s.sequence)},{"ack",int(s.ack)},{"thread",int(s.thread)},
            {"region",int(s.region.region)},{"realm",s.region.realm},{"different",int(s.region.different)},{"enter_handoff",int(s.region.enterHandoff)}});
    };
    auto startupAction=[r,&window,click,startupDwell](int waiting,int destination,const QString& action){
        r->stage=waiting;
        const auto dispatch=[r,click,waiting,destination,action]{
            if(r->done||r->stage!=waiting)return;
            r->stage=destination;click(action);
        };
        if(startupDwell)QTimer::singleShot(startupDwell,&window,dispatch);
        else dispatch();
    };
    auto input=[r,path,&window,&session,finish,mode](QString purpose){
        auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");
        if(!viewport||!viewport->isVisible()){finish(false,"Missing original viewport for "+purpose);return;}
        if(r->helper){finish(false,"Overlapping physical input for "+purpose);return;}
        const auto rect=viewport->imageRect();const auto origin=viewport->mapToGlobal(rect.topLeft().toPoint());
        auto* process=new QProcess(&window);r->process=process;r->helper=true;
        QStringList args{QDir::current().filePath(purpose=="outcome"?"tools/campaign-outcome-input.py":purpose=="realm-exit"?"tools/campaign-realm-exit-input.py":"tools/menu-storage-input.py"),
            "--rect",QString::number(origin.x()),QString::number(origin.y()),QString::number(qRound(rect.width())),QString::number(qRound(rect.height())),
            "--output",QFileInfo(path).dir().filePath(QString("progression-input-%1.json").arg(r->input++))};
        if(purpose=="outcome")args<<"--case"<<mode<<"--experiment"<<session.evidenceDirectory()<<"--seconds"<<"540"<<"--frame-size"<<QString::number(viewport->frameSize().width())<<QString::number(viewport->frameSize().height());
        else if(purpose=="realm-exit")args<<"--experiment"<<session.evidenceDirectory()<<"--thread"<<QString::number(r->thread)<<"--frame-size"<<QString::number(viewport->frameSize().width())<<QString::number(viewport->frameSize().height());
        else args<<"--mode"<<purpose<<"--input-channel"<<QDir(session.evidenceDirectory()).filePath("render-input.bin");
        QObject::connect(process,&QProcess::errorOccurred,&window,[finish](QProcess::ProcessError error){if(error==QProcess::FailedToStart)finish(false,"Input helper did not start");});
        QObject::connect(process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),&window,[r,process,finish](int code,QProcess::ExitStatus status){
            r->helper=false;r->process=nullptr;
            if(!r->done&&(code||status!=QProcess::NormalExit))finish(false,"Outcome input failed: "+QString::fromLocal8Bit(process->readAllStandardError()));
            process->deleteLater();
        });
        process->start("python3",args);
    };
    auto previous=session.stateChanged;
    session.stateChanged=[r,previous,record,click,finish,mode,probe,low,startupAction](const MenuBridge::State& s){
        r->observations.append(QJsonObject{{"ms",r->clock.elapsed()},{"stage",r->stage},{"screen",int(s.screen)},{"ready",int(s.ready)},{"ack",int(s.ack)},{"generation",int(s.generation)},{"sequence",int(s.sequence)},{"handoff",int(s.handoff)}});
        if(r->observations.size()>256)r->observations.removeAt(0);
        if(previous)previous(s);
        if(r->done||!s.ready)return;
        if(r->thread&&r->thread!=s.thread){finish(false,"Engine owner thread changed");return;}r->thread=s.thread;
        if(r->stage==0&&s.screen==3){record(s,"main");startupAction(99,100,"mainMenuAction3");}
        else if(r->stage==100&&s.screen==10){record(s,"display-configuration");r->stage=101;click(low?"preferencesRadio2":"preferencesRadio1");click("preferencesOk");}
        else if(r->stage==101&&s.screen==3){record(s,"configured-main");startupAction(102,1,"mainMenuAction0");}
        else if((r->stage==1||r->stage==7)&&s.screen==18){
            if(s.region.realm.compare("Celtic",Qt::CaseInsensitive)||s.region.region!=quint32(r->region)){finish(false,"Unexpected original region identity");return;}
            record(s,r->region==2&&s.region.enterHandoff==4?"region-travel-entry":"region-entry");r->stage=2;click("regionEntryDifficulty1");
        }else if(r->stage==2&&s.screen==18&&s.region.difficulty==1){r->stage=3;r->frames=probe().frames;r->paints=probe().paints;click("regionEntryEnter");}
        else if(r->stage==4&&s.screen==6){
            const auto expected=mode=="defeat"?0u:1u;
            record(s,"natural-report");auto row=r->states.takeAt(r->states.size()-1).toObject();QJsonArray texts;
            for(const auto& text:s.defeat.texts)texts.append(text);
            row["title"]=s.defeat.title;row["texts"]=texts;row["outcome"]=int(s.defeat.outcome);r->states.append(row);
            if(s.defeat.outcome!=expected||s.defeat.texts[19].contains("quit",Qt::CaseInsensitive)){finish(false,"Unexpected/abandoned primary outcome");return;}
            r->natural=true;r->stage=5;click("battleResultContinue");
        }else if(r->stage==5&&(s.screen==4||s.screen==3)){
            record(s,"result-return");
            if(s.screen==3){if(mode!="defeat"){finish(false,"Victory did not return Realm");return;}if(r->helper)r->stage=15;else{r->stage=12;click("mainMenuAction4");}}
            else{r->region=mode=="defeat"?1:2;r->stage=16;
                int index=-1;for(int i=0;i<s.realm.rows.size();++i)if(s.realm.rows[i].region==quint32(r->region))index=i;
                if(index<0||!(s.realm.available&(1u<<r->region))){finish(false,"Required region is not offered by original Realm");return;}
                r->realmIndex=index;}
        }else if((r->stage==3||r->stage==17)&&r->region==2&&r->visits==1&&s.screen==4){
            if(s.realm.destination==2&&s.realm.location==1){r->stage=17;return;}
            record(s,"campaign-travel");auto row=r->states.takeAt(r->states.size()-1).toObject();row["location"]=int(s.realm.location);row["destination"]=int(s.realm.destination);r->states.append(row);
            if(++r->travels!=1||s.realm.location!=2||s.realm.destination!=2){finish(false,"Original campaign travel did not settle at region2");return;}
            int index=-1;for(int i=0;i<s.realm.rows.size();++i)if(s.realm.rows[i].region==2)index=i;
            if(index<0||!(s.realm.available&(1u<<2))){finish(false,"Arrived original region2 is unavailable");return;}
            r->realmIndex=index;r->stage=16;
        }else if(r->stage==9&&s.screen==17){record(s,"cleanup-mini");r->stage=18;}
        else if(r->stage==11&&s.screen==6){record(s,"cleanup-report");r->stage=20;}
        else if(r->stage==13&&s.screen==3){record(s,"cleanup-main");r->stage=15;}
        else if(r->stage==13&&s.screen==4){
            if(s.realm.kind!=1||s.realm.location!=2){finish(false,"Unexpected cleanup Realm identity");return;}
            record(s,"cleanup-realm");auto row=r->states.takeAt(r->states.size()-1).toObject();
            row["kind"]=int(s.realm.kind);row["location"]=int(s.realm.location);row["destination"]=int(s.realm.destination);r->states.append(row);r->stage=21;
        }else if(r->stage==23&&s.screen==3){record(s,"cleanup-main");r->stage=15;}
        else if((r->stage==3||r->stage==4||r->stage==8)&&s.screen==12){record(s,"load");finish(false,"Reload is not progression");}
    };
    auto battle=session.battleStarted;
    session.battleStarted=[r,battle,finish,probe,&window](quint32 destination){
        auto* before=window.findChild<GlViewport*>("nativeGameViewport");const auto baseline=before?before->swappedWorldGeneration():0;
        if(battle)battle(destination);
        if(r->done)return;
        if(destination!=3||r->stage!=3){finish(false,"Unexpected campaign World handoff");return;}
        const auto p=probe();r->frames=p.frames;r->paints=p.paints;
        r->worldBaseline=baseline;
        r->stage=14;
    };
    auto viewport=session.originalViewportRequested;
    session.originalViewportRequested=[r,viewport]{if(viewport)viewport();if(r->done)return;if(r->stage==10)r->stage=19;else if(r->stage==3&&r->region==2)r->stage=17;};
    auto failureObserver=session.transitionFailureObserved;
    session.transitionFailureObserved=[r,failureObserver](const LiveMenuSession::TransitionDiagnostic& d){
        if(failureObserver)failureObserver(d);
        r->failure=QJsonObject{{"request",int(d.request)},{"action",int(d.action)},{"target",int(d.target)},{"flags",int(d.flags)},
            {"generation",int(d.generation)},{"screen",int(d.screen)},{"ready",int(d.ready)},{"ack",int(d.ack)},{"status",int(d.status)},
            {"sequence",int(d.sequence)},{"handoff",int(d.handoff)},{"request_age_ms",d.requestAgeMs},{"state_age_ms",d.stateAgeMs}};
    };
    auto failed=session.failed;session.failed=[failed,finish](const QString& error){if(failed)failed(error);finish(false,error);};
    auto output=session.output;
    session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto finished=session.finished;session.finished=[r,finished,finish]{if(finished)finished();finish(r->stage==12&&r->natural,"Session finished");};
    auto* timer=new QTimer(&window);timer->setInterval(100);
    QObject::connect(timer,&QTimer::timeout,&window,[r,probe,&window,&session,input,finish,low,click,deadlineMs]{
        if(r->done)return;
        const auto p=probe();
        if(r->stage==16&&!r->helper){r->stage=7;click(QString("realmViewerRegion%1").arg(r->realmIndex));click("realmViewerEnter");}
        if(r->stage==15&&!r->helper){r->stage=12;click("mainMenuAction4");}
        // A destination can become ready before the physical helper releases
        // its final key/button. Retire that helper before the next transition.
        if(r->stage==18&&!r->helper){r->stage=10;click("miniMenuButton4");}
        if(r->stage==19&&!r->helper){
            auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");
            if(viewport&&viewport->isVisible()){r->stage=11;input("yes");}
        }
        if(r->stage==20&&!r->helper){r->stage=13;click("battleResultContinue");}
        if(r->stage==21&&!r->helper){r->stage=22;if(!session.beginOriginalRealmControls()){finish(false,"Original Realm controls handoff refused");return;}}
        if(r->stage==22&&!r->helper){auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");if(viewport&&viewport->isVisible()){r->stage=23;input("realm-exit");}}
        if(p.fallback||!p.error.isEmpty()){finish(false,"Native rendering refused: "+p.error);return;}
        if(r->stage==14&&p.active&&p.frames>=r->frames+3&&p.paints>=r->paints+3){
            if(r->visits>0&&r->helper)return;
            auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");
            if(!viewport||viewport->swappedWorldGeneration()<=r->worldBaseline)return;
            if(viewport->frameSize()!=(low?QSize(640,480):QSize(800,600))){finish(false,"Actual World extent differs from requested mode");return;}
            r->worlds.append(QJsonObject{{"realm","Celtic"},{"region",r->region},{"world",qint64(viewport->swappedWorldGeneration())},
                {"after_world",qint64(r->worldBaseline)},{"width",viewport->frameSize().width()},{"height",viewport->frameSize().height()},{"paint",qint64(viewport->swappedPaint())},{"frames",qint64(p.frames)}});
            ++r->visits;
            if(r->visits==1){r->stage=4;input("outcome");}
            else if(!r->helper){r->stage=9;input("mini");}
        }
        if(r->clock.elapsed()>deadlineMs)finish(false,"Bounded campaign transition deadline exceeded");
    });timer->start();
    session.smokeSeconds=0;
    QTimer::singleShot(0,&window,[click]{click("launchGame");});
}
