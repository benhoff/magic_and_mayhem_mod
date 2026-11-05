// Interpose the real compression call to hold exactly the stage observed in
// the startup stack. No sleeps or test hooks enter the production capture.
#include "acceptance_capture.hpp"
#include <QTemporaryDir>
#include <QJsonArray>
#include <QCryptographicHash>
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
std::atomic<unsigned> compressions{0};
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

QJsonObject commandBurst(const QString& root,bool pressure,bool normalViewport=false){
    const auto directory=temporary(root);
    qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",directory.toUtf8());
    if(normalViewport)qputenv("MNM_ACCEPTANCE_NORMAL","1");
    {std::lock_guard<std::mutex> lock(gateMutex);blocked=true;entered=released=false;}
    std::promise<void> submitted;auto ready=submitted.get_future();std::string error;
    const int count=pressure?5000:1024;
    std::thread producer([&]{try{
        AcceptanceCapture capture(normalViewport?"viewport":"commands");
        for(int i=0;i<count;++i)capture.append("fragment",{{"ordinal",i}},QByteArray::number(i));
        if(!pressure){capture.append("cpu-read",{{"sequence",1}},"reply");capture.append("ended",{});}
        submitted.set_value();capture.finish();
    }catch(const std::exception& e){error=e.what();}});
    bool stageEntered;
    {std::unique_lock<std::mutex> lock(gateMutex);stageEntered=gateChanged.wait_for(lock,std::chrono::seconds(2),[]{return entered;});}
    const bool returned=ready.wait_for(std::chrono::seconds(2))==std::future_status::ready;
    {std::lock_guard<std::mutex> lock(gateMutex);released=true;}gateChanged.notify_all();producer.join();
    require(error.empty() && stageEntered,"Command burst control did not reach compressor");
    if(normalViewport)qunsetenv("MNM_ACCEPTANCE_NORMAL");
    const auto rows=read(directory);const auto close=rows.back().metadata;int fragments=0;
    bool ordered=true;
    for(const auto& row:rows)if(row.metadata["kind"]=="fragment"){
        ordered &= row.metadata["ordinal"].toInt()==fragments && row.body==QByteArray::number(fragments);++fragments;
    }
    const bool bound=rows.front().metadata["queue_limit_bytes"].toDouble()==256ll*1024*1024
        && rows.front().metadata["queue_limit_rows"].toInt()==4096;
    const bool valid=pressure?(!close["complete"].toBool() && close["error"]=="Acceptance capture queue budget exceeded" && fragments>256 && fragments<=4096):
        (close["complete"].toBool() && fragments==count && rows.size()==std::size_t(count+4)
         && rows[rows.size()-3].metadata["kind"]=="cpu-read" && rows[rows.size()-3].body=="reply"
         && rows[rows.size()-2].metadata["kind"]=="ended");
    return {{"success",returned&&bound&&ordered&&valid},{"command_fragment_burst",!normalViewport},{"normal_viewport_burst",normalViewport},{"overflow_expected",pressure},
        {"append_returned_while_compression_blocked",returned},{"full_ordered_commands_or_refusal",ordered&&valid},
        {"directory",directory},{"fragments",fragments}};
}
QJsonObject campaign(const QString& root,bool longRun){
    const auto directory=temporary(root);qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",directory.toUtf8());
    qputenv("MNM_ACCEPTANCE_CAMPAIGN","1");
    const int count=longRun?2300:192;const auto begin=compressions.load();
    QByteArray pixels(4*1024*1024,Qt::Uninitialized);auto* data=pixels.data();quint32 random=0x61a79b35;
    for(qsizetype i=0;i<pixels.size();++i){random^=random<<13;random^=random>>17;random^=random<<5;data[i]=char(random);}
    QCryptographicHash expected(QCryptographicHash::Sha256);
    {AcceptanceCapture capture("viewport");
        for(int i=0;i<count;++i){
            pixels[(i*7919)%pixels.size()]=char(i);expected.addData(pixels);
            capture.append("paint",{{"width",1024},{"height",1024},{"ordinal",i}},pixels);
            capture.append("swap",{{"paint",i+1}});
            pixels[0]=char(uchar(pixels[0])^1);
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
            while(compressions.load()<begin+unsigned(i)+1){require(std::chrono::steady_clock::now()<deadline,"Campaign compression progress");std::this_thread::yield();}
        }
        capture.finish();
    }
    qunsetenv("MNM_ACCEPTANCE_CAMPAIGN");
    const auto names=QDir(directory).entryList({"*.bin"},QDir::Files);require(names.size()==1,"Campaign wire count");
    QFile file(QDir(directory).filePath(names[0]));require(file.open(QIODevice::ReadOnly),"Campaign read");
    require(file.read(8)=="MNMACE01","Campaign envelope");
    QByteArray previous;QCryptographicHash actual(QCryptographicHash::Sha256);int rows=0,paints=0,keys=0,deltas=0,base=-1;
    QJsonObject close;QByteArray lastHash;
    while(!file.atEnd()){
        auto header=file.read(8);require(header.size()==8,"Campaign row header");
        const auto m=qFromLittleEndian<quint32>(header.constData()),n=qFromLittleEndian<quint32>(header.constData()+4);
        require(m && m<=65536 && n<=33*1024*1024,"Campaign row bounds");
        auto meta=QJsonDocument::fromJson(file.read(m)).object();auto body=n?qUncompress(file.read(n)):QByteArray{};
        require(meta["index"].toInt()==rows++,"Campaign FIFO index");
        if(meta["kind"]=="open")require(meta["version"].toInt()==2,"Campaign version");
        if(n){
            require(body.size()==pixels.size(),"Campaign decoded extent");
            if(meta["capture_body_encoding"]=="xor-previous-paint-v1"){
                require(meta["capture_body_base"].toInt()==base && previous.size()==body.size(),"Campaign base");
                auto* out=body.data();const auto* prior=previous.constData();for(qsizetype i=0;i<body.size();++i)out[i]=char(uchar(out[i])^uchar(prior[i]));++deltas;
            }else{require(meta["capture_body_encoding"]=="full-rgba-v1" && meta["capture_body_base"].toInt()==-1,"Campaign full key");++keys;}
            lastHash=QCryptographicHash::hash(body,QCryptographicHash::Sha256).toHex();require(meta["capture_body_sha256"].toString().toLatin1()==lastHash,"Campaign plane hash");
            actual.addData(body);previous=body;base=meta["index"].toInt();++paints;
        }
        close=meta;
    }
    const bool success=close["complete"].toBool() && paints==count && actual.result()==expected.result()
        && keys==(count+127)/128 && deltas==count-keys && file.size()<128ll*1024*1024
        && close["raw_bytes"].toString().toLongLong()==qint64(pixels.size())*count;
    return {{"success",success},{"directory",directory},{"campaign",true},{"paints",paints},{"keyframes",keys},{"delta_paints",deltas},
        {"decoded_bytes",QString::number(qint64(pixels.size())*count)},{"stored_bytes",double(file.size())},
        {"last_plane_sha256",QString::fromLatin1(lastHash)},{"every_snapshot_matches_source",actual.result()==expected.result()}};
}
}

QByteArray qCompress(const uchar* source,qsizetype length,int level){
    ++compressions;
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
    qputenv("MNM_ACCEPTANCE_CAPTURE_ID","capture-control");qunsetenv("MNM_ACCEPTANCE_NORMAL");qunsetenv("MNM_ACCEPTANCE_CAMPAIGN");
    const bool baseline=argc==3 && QByteArray(argv[2])=="--baseline";
    const bool longRun=argc==3 && QByteArray(argv[2])=="--campaign-long";
    const QString root=QString::fromLocal8Bit(argv[1]);QDir().mkpath(root);
    QJsonArray cases;cases.append(gated(root,false));
    if(!baseline){
        cases.append(gated(root,true));
        cases.append(commandBurst(root,false));cases.append(commandBurst(root,true));
        cases.append(commandBurst(root,false,true));cases.append(commandBurst(root,true,true));
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
        cases.append(campaign(root,longRun));
    }
    bool success=true;for(const auto& value:cases)success&=value.toObject()["success"].toBool();
    std::cout<<QJsonDocument(QJsonObject{{"success",success},{"baseline",baseline},{"cases",cases}}).toJson(QJsonDocument::Compact).constData()<<'\n';
    return success?0:1;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
