#include "live_menu_session.hpp"
#include "voice_bridge.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QTemporaryDir>
#include <QtEndian>
#include <QThread>
#include <iostream>
#include <stdexcept>

void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void script(const QString& path,const QByteArray& body){
    QFile file(path);check(file.open(QIODevice::WriteOnly),"script open");
    check(file.write("#!/usr/bin/python3\n"+body)>0,"script write");file.close();
    check(file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner),"script mode");
}
struct LiveMenuSessionTestAccess {
    static void mediaLease(LiveMenuSession& session){
        session.clock_.start();session.lastState_=session.requestedAt_=0;
        session.setMediaActive(true);QThread::msleep(20);session.setMediaActive(false);
        check(session.lastState_>=15&&session.requestedAt_==session.lastState_,"movie pause preserved both menu lease clocks");
        const auto saved=session.lastState_;session.setMediaActive(false);
        check(session.lastState_==saved,"repeated movie completion did not extend lease");
    }
};
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);unsigned launched=0,refused=0;
    for(unsigned variant=0;variant<7;++variant){
        QTemporaryDir repo;check(repo.isValid(),"temporary repo");
        check(QDir().mkpath(repo.filePath("tools")),"tools directory");
        check(QDir().mkpath(repo.filePath("evidence")),"evidence directory");
        script(repo.filePath("tools/prepare-menu-observer.py"),
            "import pathlib,sys\nr=pathlib.Path.cwd()\n(r/'stage.args').write_text(' '.join(sys.argv[1:]))\nprint('Evidence directory: '+str(r/'evidence'),flush=True)\n");
        script(repo.filePath("tools/run-menu-observer.py"),
            "import os,pathlib,struct,sys\nr=pathlib.Path.cwd()\nassert 'MNM_AUDIO_CHANNEL' not in os.environ\nassert 'MNM_RENDER_MEDIA' not in os.environ\np=r/'evidence/native-voices.bin'\nif p.exists():\n with p.open('rb') as f: h=f.read(128)\n assert h[:8]==b'MNMAUD01' and struct.unpack_from('<I',h,80)[0]==1\nif '--native-media' in (r/'stage.args').read_text():\n assert (r/'evidence/native-media.bin').is_file()\n(r/'launched').write_text(' '.join(sys.argv[1:]))\n");
        qputenv("MNM_AUDIO_CHANNEL","unapproved inherited channel");
        qputenv("MNM_RENDER_MEDIA","unapproved inherited media");
        qputenv("MNM_MENU_GAME_SOURCE",repo.filePath("source-game").toUtf8());
        LiveMenuSession session(repo.path());mnm::audio::VoiceBroker broker;
        LiveMenuSessionTestAccess::mediaLease(session);
        session.preferencesMenusEnabled=false;session.nativeCommandsEnabled=true;
        session.nativeVoicesEnabled=variant!=3;
        session.nativeMediaEnabled=variant>=4;
        session.prepareNativePresentation=[](const QString&){return true;};
        unsigned prepared=0;bool failed=false,done=false,timedOut=false;
        if(variant!=2)session.prepareNativeAudio=[&](const QString& path){
            ++prepared;check(path==repo.filePath("evidence/native-voices.bin"),"owned channel path");
            return variant!=1&&broker.create(path,false);
        };
        unsigned mediaPrepared=0;
        if(variant!=6)session.prepareNativeMedia=[&](const QString& path){
            ++mediaPrepared;check(path==repo.filePath("evidence/native-media.bin"),"owned media channel path");
            if(variant==5)return false;
            QFile file(path);return file.open(QIODevice::WriteOnly|QIODevice::NewOnly)&&file.resize(2048);
        };
        session.failed=[&](const QString&){failed=true;};
        QEventLoop loop;session.finished=[&]{broker.stop();done=true;loop.quit();};
        QTimer deadline;deadline.setSingleShot(true);QObject::connect(&deadline,&QTimer::timeout,&loop,[&]{timedOut=true;loop.quit();});deadline.start(5000);
        check(session.start(),"session start");loop.exec();check(done&&!timedOut&&!session.running(),"bounded completion");
        QFile staged(repo.filePath("stage.args"));check(staged.open(QIODevice::ReadOnly),"stage receipt");
        const auto stageArguments=staged.readAll();
        check(stageArguments.contains("--native-voices")==session.nativeVoicesEnabled,"voice staging policy");
        check(stageArguments.contains("--native-media")==session.nativeMediaEnabled,"media staging policy");
        check(stageArguments.contains("--source-game "+repo.filePath("source-game").toUtf8()),"isolated source forwarded");
        const bool expected=variant==0||variant==3||variant==4;
        check(QFile::exists(repo.filePath("launched"))==expected,"launcher admission");
        check(failed!=expected,"failure propagated");
        check(prepared==(variant!=2&&variant!=3?1u:0u),"audio prepared once before launch");
        check(mediaPrepared==(variant==4||variant==5?1u:0u),"media prepared once before launch");
        if(variant==0){QFile channel(repo.filePath("evidence/native-voices.bin"));check(channel.open(QIODevice::ReadOnly),"retired channel");const auto bytes=channel.read(128);check(qFromLittleEndian<quint32>(bytes.constData()+80)==0,"finish withdrew readiness");}
        expected?++launched:++refused;
    }
    qunsetenv("MNM_AUDIO_CHANNEL");
    qunsetenv("MNM_RENDER_MEDIA");
    qunsetenv("MNM_MENU_GAME_SOURCE");
    std::cout<<"Native menu audio lifecycle: "<<launched<<" launches, "<<refused<<" refusals; no game or output device\n";return 0;
} catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
