// Interpose the real compression call to hold exactly the stage observed in
// the startup stack. No sleeps or test hooks enter the production capture.
#include "acceptance_capture.hpp"
#include <QTemporaryDir>
#include <QJsonArray>
#include <zlib.h>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
std::mutex gateMutex;
std::condition_variable gateChanged;
bool blocked=false,entered=false,released=false,compressionFailure=false;
std::thread::id compressorThread;
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
struct Row {QJsonObject metadata;QByteArray body;};
std::vector<Row> read(const QString& directory){
    const auto names=QDir(directory).entryList({"*.bin"},QDir::Files);
    require(names.size()==1,"Expected one capture");
    QFile file(QDir(directory).filePath(names[0]));require(file.open(QIODevice::ReadOnly),"Read capture");
    const auto raw=file.readAll();require(raw.left(8)=="MNMACE01","Envelope");
    qsizetype at=8;std::vector<Row> rows;qint64 bytes=0;qint64 lastTime=0;
    while(at<raw.size()){
        require(raw.size()-at>=8,"Truncated row");
        const auto m=qFromLittleEndian<quint32>(raw.constData()+at),n=qFromLittleEndian<quint32>(raw.constData()+at+4);at+=8;
        require(m && m<=65536 && qint64(m)+n<=raw.size()-at,"Row bounds");
        const auto meta=QJsonDocument::fromJson(raw.mid(at,m)).object();at+=m;
        const auto body=n?qUncompress(raw.mid(at,n)):QByteArray{};at+=n;
        require(!n || body.size()==qFromBigEndian<quint32>(raw.constData()+at-n),"Compressed body");
        require(meta["index"].toInt()==int(rows.size()),"FIFO index");
        const auto time=meta["host_us"].toString().toLongLong();require(time>=lastTime,"Clock order");lastTime=time;
        bytes+=body.size();rows.push_back({meta,body});
    }
    require(rows.size()>=2 && rows.front().metadata["kind"]=="open" && rows.back().metadata["kind"]=="close","Closed lifetime");
    require(rows.back().metadata["prior_rows"].toInt()==int(rows.size()-1),"Terminal row count");
    require(rows.back().metadata["raw_bytes"].toString().toLongLong()==bytes,"Terminal byte count");
    return rows;
}
QString temporary(const QString& root){
    QTemporaryDir directory(QDir(root).filePath("capture-XXXXXX"));require(directory.isValid(),"Temporary directory");
    directory.setAutoRemove(false);return directory.path();
}
QJsonObject gated(const QString& root,bool pressure){
    const auto directory=temporary(root);
    qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",directory.toUtf8());
    {std::lock_guard<std::mutex> lock(gateMutex);blocked=true;entered=released=false;}
    std::promise<void> submitted;auto ready=submitted.get_future();std::string error;
    std::thread::id producerThread;
    std::thread producer([&]{try{
        producerThread=std::this_thread::get_id();
        AcceptanceCapture capture("viewport");
        QByteArray storage(pressure?16*1024*1024:1024,'a');
        const auto borrowed=QByteArray::fromRawData(storage.constData(),storage.size());
        capture.append("paint",{{"source","before"}},borrowed);
        // This storage changes before the worker is permitted to compress it.
        storage[0]='z';
        if(pressure)for(int i=0;i<32;++i)capture.append("paint",{{"ordinal",i}},storage);
        else capture.append("swap",{{"paint",1}});
        submitted.set_value();capture.finish();capture.finish();
    }catch(const std::exception& e){error=e.what();}});
    bool stageEntered;
    {std::unique_lock<std::mutex> lock(gateMutex);stageEntered=gateChanged.wait_for(lock,std::chrono::seconds(2),[]{return entered;});}
    const bool returned=ready.wait_for(std::chrono::seconds(2))==std::future_status::ready;
    {std::lock_guard<std::mutex> lock(gateMutex);released=true;}gateChanged.notify_all();producer.join();
    require(error.empty(),"Capture producer exception");require(stageEntered,"Compression gate not reached");
    const auto rows=read(directory);const auto close=rows.back().metadata;
    const bool offThread=compressorThread!=producerThread;
    bool valid;
    if(pressure)valid=!close["complete"].toBool() && close["error"]=="Acceptance capture queue budget exceeded" && rows.size()<35;
    else valid=close["complete"].toBool() && close["error"].toString().isEmpty() && rows.size()==4 && rows[1].body==QByteArray(1024,'a') && rows[1].metadata["source"]=="before" && rows[2].metadata["kind"]=="swap";
    return {{"success",returned&&offThread&&valid},{"append_returned_while_compression_blocked",returned},
        {"compression_off_caller_thread",offThread},{"ordered_owned_payloads_or_bounded_refusal",valid},
        {"directory",directory},{"rows",int(rows.size())}};
}
}

QByteArray qCompress(const uchar* source,qsizetype length,int level){
    {std::unique_lock<std::mutex> lock(gateMutex);
        compressorThread=std::this_thread::get_id();entered=true;gateChanged.notify_all();
        if(blocked)gateChanged.wait(lock,[]{return released;});
        if(compressionFailure)return {};
    }
    uLongf size=compressBound(uLong(length));QByteArray result(qsizetype(size)+4,0);
    qToBigEndian<quint32>(quint32(length),result.data());
    require(compress2(reinterpret_cast<Bytef*>(result.data()+4),&size,source,uLong(length),level)==Z_OK,"Compression backend");
    result.resize(qsizetype(size)+4);return result;
}

int main(int argc,char** argv)try{
    require(argc==2 || argc==3,"Expected owned output directory and optional --baseline");
    qputenv("MNM_ACCEPTANCE_CAPTURE_ID","capture-control");qunsetenv("MNM_ACCEPTANCE_NORMAL");
    const bool baseline=argc==3 && QByteArray(argv[2])=="--baseline";
    const QString root=QString::fromLocal8Bit(argv[1]);QDir().mkpath(root);
    QJsonArray cases;cases.append(gated(root,false));
    if(!baseline){
        cases.append(gated(root,true));
        {std::lock_guard<std::mutex> lock(gateMutex);blocked=false;compressionFailure=true;}
        const auto directory=temporary(root);qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",directory.toUtf8());
        {AcceptanceCapture capture("viewport");capture.append("paint",{},"x");capture.finish();}
        auto rows=read(directory);auto terminal=rows.back().metadata;
        const bool refused=!terminal["complete"].toBool() && terminal["error"]=="Acceptance capture compression failed";
        cases.append(QJsonObject{{"success",refused},{"compression_failure_refused",refused}});
        {std::lock_guard<std::mutex> lock(gateMutex);compressionFailure=false;}
        const auto failureDirectory=temporary(root);qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",failureDirectory.toUtf8());
        {AcceptanceCapture capture("viewport");capture.fail("first refusal");capture.fail("later refusal");capture.append("paint",{},"ignored");capture.finish();}
        rows=read(failureDirectory);terminal=rows.back().metadata;
        const bool failed=!terminal["complete"].toBool() && terminal["error"]=="first refusal" && rows.size()==2;
        cases.append(QJsonObject{{"success",failed},{"first_failure_preserved",failed}});
        qunsetenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY");{AcceptanceCapture disabled("viewport");require(!disabled.enabled(),"Disabled capture");disabled.append("paint",{},"unused");}
    }
    bool success=true;for(const auto& value:cases)success&=value.toObject()["success"].toBool();
    std::cout<<QJsonDocument(QJsonObject{{"success",success},{"baseline",baseline},{"cases",cases}}).toJson(QJsonDocument::Compact).constData()<<'\n';
    return success?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
