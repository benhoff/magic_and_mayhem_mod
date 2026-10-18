#include "native_only_session.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <cassert>
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir directory;assert(directory.isValid());
    NativeOnlySession session;assert(session.begin(directory.path()));
    assert(session.finish(true,0,true,false));assert(session.completed());
    NativeOnlySession duplicate;assert(!duplicate.begin(directory.path()));assert(duplicate.fail("duplicate must not overwrite evidence"));
    QFile accepted(QDir(directory.path()).filePath("native-only-session.json"));assert(accepted.open(QIODevice::ReadOnly));
    assert(QJsonDocument::fromJson(accepted.readAll()).object()["state"]=="completed");
    for(int failure=0;failure<5;++failure){
        QTemporaryDir run;NativeOnlySession s;assert(s.begin(run.path()));
        if(failure==0)assert(s.fail("first failure"));
        assert(s.finish(failure!=1,failure==2?124:0,failure!=3,failure==4));assert(!s.completed());
        assert(s.finish(true,0,true,false));assert(!s.completed());
        assert(s.fail("second failure"));
        QFile record(QDir(run.path()).filePath("native-only-session.json"));assert(record.open(QIODevice::ReadOnly));
        auto json=QJsonDocument::fromJson(record.readAll()).object();assert(json["state"]=="failed");
        if(failure==0)assert(json["error"]=="first failure");
    }
    for(bool failed:{false,true}){
        QTemporaryDir first,second;NativeOnlySession s;assert(s.begin(first.path()));
        assert(!s.begin(second.path()));if(failed)assert(s.fail("retained old failure"));
        assert(s.requestRestart());assert(s.retireForRestart(true,true));
        QFile old(QDir(first.path()).filePath("native-only-session.json"));assert(old.open(QIODevice::ReadOnly));
        const auto bytes=old.readAll();const auto retired=QJsonDocument::fromJson(bytes).object();
        assert(retired["state"]=="restarted"&&retired["force_stop_verified"]==true&&retired["renderer_retired"]==true);
        assert(retired["error"]==(failed?"retained old failure":""));
        assert(s.begin(second.path()));assert(s.finish(true,0,true,false));assert(s.completed());
        old.seek(0);assert(old.readAll()==bytes);
    }
    for(int failure=0;failure<3;++failure){
        QTemporaryDir run,next;NativeOnlySession s;assert(s.begin(run.path()));
        if(failure)assert(s.requestRestart());
        assert(s.retireForRestart(failure!=1,failure!=2));assert(!s.begin(next.path()));
    }
    {QTemporaryDir old,next;NativeOnlySession s;assert(s.begin(old.path()));
     assert(s.requestRestart()&&s.retireForRestart(true,true));
     QFile file(QDir(old.path()).filePath("native-only-session.json"));assert(file.open(QIODevice::ReadOnly));const auto bytes=file.readAll();
     assert(s.fail("new staging failed"));file.seek(0);assert(file.readAll()==bytes);
     assert(s.requestRestart()&&s.retireForRestart(true,true));assert(s.begin(next.path()));
     assert(s.finish(true,0,true,false));}
    return 0;
}
