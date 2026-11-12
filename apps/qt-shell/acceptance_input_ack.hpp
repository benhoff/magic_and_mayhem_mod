#pragma once
#include <QFile>
#include <QByteArray>
#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QtEndian>
#include <functional>
#include <memory>

// Acceptance-driver policy over the existing MNMIAK01 observation journal.
// Reading this receipt never synthesizes an original poll or renews a lease.
struct AcceptanceInputAck {
    static constexpr int DeadlineMs=3000;
    static unsigned transition(const QByteArray& data,unsigned after,quint32 published,bool down){
        if(data.size()<64 || data.size()>64+7*48 || data.left(8)!="MNMIAK01" || !published || (published&1))return 0;
        auto word=[&](int at){return qFromLittleEndian<quint32>(data.constData()+at*4);};
        const auto count=word(4);
        if(word(2)!=1 || word(3)!=48 || word(5)!=8 || !count || count>=8 || data.size()!=64+int(count)*48)return 0;
        for(int i=6;i<16;++i)if(word(i))return 0;
        for(unsigned i=0;i<count;++i){
            const int at=16+int(i)*12;
            if(word(at)!=i+1 || word(at+2)!=39 || word(at+3)>65535 || word(at+4)>1 ||
               word(at+5)<0x400000 || word(at+5)>=0x5d0000 || !word(at+6) ||
               bool(word(at+3)&0x8000)!=(i%2==0))return 0;
            for(int j=8;j<12;++j)if(word(at+j))return 0;
        }
        if(after>=count)return 0;
        const int at=16+int(after)*12;
        if(bool(word(at+3)&0x8000)!=down)return 0;
        // A press must be the held owned publication. A fallback release is
        // legitimate after focus loss, but must follow this observed press.
        if(down && (word(at+4)!=1 || word(at+1)<published))return 0;
        if(!down && word(at+4)==1 && word(at+1)<published)return 0;
        return after+1;
    }
    static void wait(QObject* owner,const QString& path,unsigned after,quint32 published,bool down,
                     std::function<void(unsigned)> matched,std::function<void()> expired){
        auto* timer=new QTimer(owner);timer->setInterval(20);
        auto elapsed=std::make_shared<QElapsedTimer>();elapsed->start();
        QObject::connect(timer,&QTimer::timeout,owner,[timer,elapsed,path,after,published,down,matched,expired]{
            QFile file(path);QByteArray bytes;if(file.open(QIODevice::ReadOnly))bytes=file.read(64+8*48+1);
            const auto row=transition(bytes,after,published,down);
            if(elapsed->elapsed()>=DeadlineMs){timer->stop();timer->deleteLater();expired();}
            else if(row){timer->stop();timer->deleteLater();matched(row);}
        });
        timer->start();
    }
};
