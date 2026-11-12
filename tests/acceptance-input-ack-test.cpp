#include "acceptance_input_ack.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>
#include <stdexcept>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static QByteArray journal(unsigned count){
    QByteArray bytes(64+int(count)*48,0);
    auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes.data()+word*4);};
    std::copy_n("MNMIAK01",8,bytes.data());put(2,1);put(3,48);put(4,count);put(5,8);
    for(unsigned i=0;i<count;++i){int at=16+int(i)*12;put(at,i+1);put(at+1,100+20*i);put(at+2,39);put(at+3,i%2?0:32769);put(at+4,1);put(at+5,0x469b9b);put(at+6,312);}
    return bytes;
}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QJsonObject result{{"success",false}};
    try{
        check(argc==2,"Output path required");const auto root=QString::fromLocal8Bit(argv[1]);check(QDir().mkpath(root),"Output mkdir");
        check(!AcceptanceInputAck::transition({},0,100,true),"Empty receipt");
        auto one=journal(1),two=journal(2),four=journal(4);
        check(AcceptanceInputAck::transition(one,0,100,true)==1,"Owned current press");
        check(!AcceptanceInputAck::transition(one,0,102,true),"Older publication refused");
        check(!AcceptanceInputAck::transition(two,2,140,true),"Previous World cannot authorize new press");
        check(AcceptanceInputAck::transition(four,2,140,true)==3,"Fresh second press");
        check(AcceptanceInputAck::transition(four,3,160,false)==4,"Fresh second release");
        check(!AcceptanceInputAck::transition(four,3,162,false),"Stale owned release refused");
        qToLittleEndian<quint32>(0,two.data()+(16+12+1)*4);qToLittleEndian<quint32>(0,two.data()+(16+12+4)*4);
        check(AcceptanceInputAck::transition(two,1,120,false)==2,"Observed fallback release after focus loss");
        for(int size=0;size<one.size();++size)check(!AcceptanceInputAck::transition(one.left(size),0,100,true),"Partial write refused");
        for(int field:{2,3,4,5,6,16,18,20,21,24}){
            auto bad=one;qToLittleEndian<quint32>(0xffffffffu,bad.data()+field*4);
            check(!AcceptanceInputAck::transition(bad,0,100,true),"Malformed receipt refused");
        }
        {auto bad=one;qToLittleEndian<quint32>(0,bad.data()+22*4);check(!AcceptanceInputAck::transition(bad,0,100,true),"Missing original thread refused");}
        check(!AcceptanceInputAck::transition(journal(8),0,100,true),"Full journal cannot prove closure");
        result["wire_identity_and_partial_write_controls"]=true;
        // The actual original poll can arrive after the former 450 ms hold.
        // Keep the semantic key held until its observed receipt, then await release.
        const auto path=QDir(root).filePath("receipt.bin");bool held=true,pressed=false,released=false,expired=false;int ticks=0;
        QTimer progress;progress.setInterval(5);QObject::connect(&progress,&QTimer::timeout,&app,[&]{++ticks;});progress.start();
        QTimer::singleShot(500,&app,[&]{QFile file(path);check(file.open(QIODevice::WriteOnly),"Partial journal open");});
        QTimer::singleShot(750,&app,[&]{check(held,"Caller released before original poll");QFile file(path);check(file.open(QIODevice::WriteOnly),"Press open");check(file.write(one)==one.size(),"Press write");});
        AcceptanceInputAck::wait(&app,path,0,100,true,[&](unsigned row){
            check(row==1&&held,"Fresh press callback");held=false;pressed=true;
            AcceptanceInputAck::wait(&app,path,row,120,false,[&](unsigned releasedRow){check(releasedRow==2,"Release callback");released=true;app.quit();},[&]{expired=true;app.quit();});
        },[&]{expired=true;app.quit();});
        QTimer::singleShot(1000,&app,[&]{check(!held&&pressed,"Release scheduled after original press");QFile file(path);check(file.open(QIODevice::WriteOnly),"Release open");check(file.write(two)==two.size(),"Release write");});
        app.exec();check(pressed&&released&&!expired&&ticks>100,"Nonblocking original acknowledgement wait");
        result["poll_after_old_hold_window_observed_before_release"]=true;result["event_loop_ticks_during_wait"]=ticks;
        bool falseAdvance=false;QElapsedTimer deadline;deadline.start();
        AcceptanceInputAck::wait(&app,path,2,140,true,[&](unsigned){falseAdvance=true;app.quit();},[&]{expired=true;app.quit();});
        app.exec();check(expired&&!falseAdvance&&deadline.elapsed()>=AcceptanceInputAck::DeadlineMs&&deadline.elapsed()<4500,"Missing original poll is bounded refusal");
        result["missing_poll_refused_without_retry"]=true;result["wait_deadline_ms"]=AcceptanceInputAck::DeadlineMs;result["success"]=true;
    }catch(const std::exception& error){result["error"]=QString::fromLocal8Bit(error.what());}
    std::cout<<QJsonDocument(result).toJson(QJsonDocument::Compact).constData()<<'\n';return result.value("success").toBool()?0:1;
}
