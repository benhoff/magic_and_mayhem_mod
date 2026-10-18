#pragma once
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <functional>
#include <csignal>
#include <unistd.h>

// Each launcher has its own process group. Wine may detach processes from it,
// so a running game also needs a prefix-scoped wineserver stop and wait.
class GameProcess final : public QProcess {
public:
    GameProcess(){
        setUnixProcessParameters(UnixProcessFlag::CreateNewSession);
        connect(this,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this]{QTimer::singleShot(0,this,[this]{complete();});});
        connect(&server_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus status){
            if(!stopping_)return;
            if(timedOut_){serverDone_=true;complete();return;}
            const auto diagnostic=QString::fromLocal8Bit(server_.readAllStandardError());
            // Wine -k exits1 without diagnostics when the namespace's server
            // already disappeared. Still require the subsequent -w barrier.
            const bool absent=!waitingServer_&&code==1&&diagnostic.isEmpty();
            if((code!=0&&!absent)||status!=QProcess::NormalExit){error_="Wine shutdown failed: "+diagnostic;serverDone_=true;complete();return;}
            if(waitingServer_){serverDone_=true;complete();}
            else{waitingServer_=true;server_.start(serverProgram_,{"-w"});}
        });
        connect(&server_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){
            if(stopping_&&error==QProcess::FailedToStart){error_="Cannot stop Wine: "+server_.errorString();serverDone_=true;complete();}
        });
        timeout_.setSingleShot(true);timeout_.setInterval(10000);
        connect(&timeout_,&QTimer::timeout,this,[this]{
            error_="Game shutdown timed out; restart was cancelled.";
            timedOut_=true;server_.kill();serverDone_=true;complete();
        });
    }
    bool hardStopping() const{return stopping_;}
    void forceStop(const QString& prefix,std::function<void(bool,const QString&)> callback,const QString& serverProgram=QStringLiteral("wineserver")){
        if(stopping_)return;
        stopping_=true;done_=std::move(callback);error_.clear();waitingServer_=timedOut_=false;
        serverDone_=prefix.isEmpty();serverProgram_=serverProgram;
        // QProcess owns/reaps the leader. Kill descendants even if the leader
        // itself would respond to TERM; a frozen engine must not veto recovery.
        const auto pid=processId();if(pid>0)::kill(-pid,SIGKILL);
        kill();timeout_.start();
        if(!prefix.isEmpty()){
            auto env=processEnvironment();if(env.isEmpty())env=QProcessEnvironment::systemEnvironment();
            env.insert("WINEPREFIX",prefix);server_.setProcessEnvironment(env);
            server_.start(serverProgram_,{"-k"});
        }
        QTimer::singleShot(0,this,[this]{complete();});
    }
private:
    void complete(){
        if(!stopping_||!serverDone_||server_.state()!=QProcess::NotRunning||(state()!=QProcess::NotRunning&&!timedOut_))return;
        timeout_.stop();stopping_=false;auto callback=std::move(done_);
        if(callback)callback(error_.isEmpty(),error_);
    }
    QProcess server_;QTimer timeout_;bool stopping_=false,serverDone_=false,waitingServer_=false,timedOut_=false;
    QString error_,serverProgram_;std::function<void(bool,const QString&)> done_;
};
