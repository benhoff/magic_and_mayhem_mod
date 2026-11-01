#pragma once
#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <chrono>
#include <cstdio>

// Explicit diagnostic only. Freeze the GUI callback while service workers
// continue. Existing producer deadlines and ownership checks remain unchanged.
class WatchdogHostFault final {
public:
    static void setCommandPath(const QString& path){
        if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_STALL_WORLD_HOST")==1)commandPath()=path;
    }
    static void afterMovieFrame(QObject* owner,const QString& mediaPath){
        schedule(owner,mediaPath,"MNM_ACCEPTANCE_STALL_INTRO_HOST","intro",1000);
    }
    static void afterWorldPaint(QObject* owner){
        schedule(owner,commandPath(),
                 "MNM_ACCEPTANCE_STALL_WORLD_HOST","world",0);
    }
private:
    static QString& commandPath(){static QString path;return path;}
    static void schedule(QObject* owner,const QString& path,const char* flag,const char* target,int delay){
        static bool scheduled=false;
        if(scheduled || qEnvironmentVariableIntValue("MNM_ACCEPTANCE_NORMAL")!=1 ||
           qEnvironmentVariableIntValue(flag)!=1)return;
        scheduled=true;
        QTimer::singleShot(delay,owner,[path,target]{
            const auto begin=now();
            const auto record=[&](const char* phase){
                QFile file(path);QByteArray bytes;
                // Media is 2048 bytes. A ring snapshot needs only its fixed
                // 64-byte header; its complete final bytes are retained later.
                const bool media=QByteArray(target)=="intro";
                if(file.open(QIODevice::ReadOnly) && file.size()==(media?2048:1048640))bytes=file.read(media?2048:64);
                const auto row=QJsonDocument(QJsonObject{{"phase",QString::fromLatin1(phase)},
                    {"target",QString::fromLatin1(target)},{"begin_host_us",QString::number(begin)},
                    {"observed_host_us",QString::number(now())},{"planned_ms",6200},
                    {"snapshot_hex",QString::fromLatin1(bytes.toHex())}}).toJson(QJsonDocument::Compact);
                std::fprintf(stderr,"NATIVE_HOST_FAULT %s\n",row.constData());
            };
            record("begin");QThread::msleep(6200);record("end");
        });
    }
    static qint64 now(){return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();}
};
