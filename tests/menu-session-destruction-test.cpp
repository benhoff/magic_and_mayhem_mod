#include "live_menu_session.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QElapsedTimer>
#include <QThread>
#include <iostream>
#include <memory>
#include <stdexcept>

struct LiveMenuSessionTestAccess {
    static qint64 start(LiveMenuSession& session,const QString& path,bool staging){
        session.preparing_=staging;
        session.bridge_=std::make_unique<MenuBridge>();
        if(!session.bridge_->create(path))throw std::runtime_error("bridge creation");
        session.process_.start("/usr/bin/sleep",{"30"});
        if(!session.process_.waitForStarted())throw std::runtime_error("fixture process startup");
        return session.process_.processId();
    }
};

static bool alive(qint64 pid){
    QFile file(QString("/proc/%1/stat").arg(pid));
    if(!file.open(QIODevice::ReadOnly))return false;
    const auto data=file.readAll(),state=data.mid(data.lastIndexOf(')')+2,1);
    return state!="Z"&&state!="X";
}
static void requireDead(qint64 pid){
    QElapsedTimer clock;clock.start();
    while(alive(pid)&&clock.elapsed()<2000)QThread::msleep(2);
    if(alive(pid))throw std::runtime_error("owned child survived destruction");
}

int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);QTemporaryDir root;
    if(!root.isValid())throw std::runtime_error("fixture directory");
    unsigned callbacks=0;
    const QByteArray mode=argc>1?argv[1]:"active";
    if(mode=="process"||mode=="stop"){
        auto process=std::make_unique<GameProcess>();
        QObject::connect(process.get(),qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&]{++callbacks;});
        process->start("/usr/bin/python3",{"-c","import os,time; p=os.fork(); print(p,flush=True) if p else None; time.sleep(30)"});
        if(!process->waitForStarted()||!process->waitForReadyRead())throw std::runtime_error("process tree startup");
        const auto child=process->readAllStandardOutput().trimmed().toLongLong(),leader=process->processId();
        if(child<=0||!alive(child))throw std::runtime_error("descendant fixture startup");
        if(mode=="stop"){
            const auto path=root.filePath("server");QFile helper(path);
            if(!helper.open(QIODevice::WriteOnly)||helper.write("#!/usr/bin/python3\nimport time\ntime.sleep(30)\n")<0)throw std::runtime_error("helper fixture");
            helper.close();helper.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner);
            process->forceStop(root.filePath("prefix"),[&](bool,const QString&){++callbacks;},path);
        }
        process.reset();QCoreApplication::processEvents();requireDead(leader);requireDead(child);
        if(callbacks)throw std::runtime_error("callbacks escaped process destruction");
        std::cout<<"PASS: "<<mode.constData()<<" destruction reaps process tree without callbacks\n";return 0;
    }
    auto session=std::make_unique<LiveMenuSession>(root.path());
    session->failed=[&](const QString&){++callbacks;};
    session->finished=[&]{++callbacks;};
    session->output=[&](const QString&){++callbacks;};
    const bool staging=mode=="staging";
    const auto pid=LiveMenuSessionTestAccess::start(*session,root.filePath("menu.bin"),staging);
    session.reset();QCoreApplication::processEvents();requireDead(pid);
    if(callbacks)throw std::runtime_error("callbacks escaped session destruction");
    std::cout<<"PASS: active "<<(staging?"staging":"launcher")<<" destruction has no callbacks\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
