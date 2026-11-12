#include "../apps/qt-shell/resident_profile_capture.hpp"
#include "../apps/qt-shell/profile_capture_codec.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <chrono>
#include <iostream>
#include <thread>
using namespace mnm::profile;
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);check(argc==2,"Expected owned fixture directory");
    const QString root=argv[1];check(QDir().mkpath(root),"Fixture output");qputenv("MNM_ACCEPTANCE_CAMPAIGN","1");
    std::vector<std::uint8_t> source(1024*1024),request(64,9);quint32 random=0x5198ab29;
    for(auto& byte:source){random^=random<<13;random^=random>>17;random^=random<<5;byte=std::uint8_t(random);}
    ResidentProfileCapture capture(root+"/sources-v4.bin",256ull*1024*1024,true);
    QJsonArray expected;quint64 chunks=0,decoded=0;
    auto append=[&](unsigned generation,bool chunk){
        auto body=source;if(chunk)body.resize(512*1024);
        QByteArray raw(reinterpret_cast<const char*>(body.data()),qsizetype(body.size()));
        if(!chunk)raw.append(reinterpret_cast<const char*>(request.data()),qsizetype(request.size()));
        expected.append(QJsonObject{{"generation",int(generation)},{"chunk",chunk?int(++chunks):0},
            {"source",int(body.size())},{"request",chunk?0:int(request.size())},
            {"sha256",QString::fromLatin1(QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex())}});
        decoded+=raw.size();check(chunk?capture.enqueueSource(generation,body):capture.enqueue(generation,body,request),"Source fixture enqueue");
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
        while(capture.report()["queue_bytes"].toDouble()>4*1024*1024){
            check(capture.report()["error"].toString().isEmpty() && std::chrono::steady_clock::now()<deadline,"Source fixture compression progress");std::this_thread::yield();}
    };
    for(unsigned generation=1;generation<=300;++generation){
        std::rotate(source.begin(),source.begin()+37,source.end());source[generation*97%source.size()]=std::uint8_t(generation);
        if(generation%7==0)source.push_back(std::uint8_t(generation));
        if(generation%17==0)append(generation,true);
        append(generation,false);
    }
    capture.finish();const auto report=capture.report();check(report["complete"].toBool() && report["format"].toInt()==4,"Source fixture complete V4");
    QFile file(root+"/sources-v4.bin");check(file.open(QIODevice::ReadOnly) && file.read(8)=="MNMPROF4","Source fixture archive");
    Decoder decoder;unsigned index=0;QByteArray firstHeader,firstPacked;
    while(!file.atEnd()){
        const auto header=file.read(HeaderSize);check(header.size()==HeaderSize,"Source fixture header");
        const auto n=qFromLittleEndian<quint32>(header.constData()+24);const auto packed=file.read(n);
        const auto s=qFromLittleEndian<quint32>(header.constData()+16),r=qFromLittleEndian<quint32>(header.constData()+20);
        const auto body=decoder.decode(header,packed,s+r);const auto entry=expected[index++].toObject();
        check(qFromLittleEndian<quint64>(header.constData())==quint64(entry["generation"].toInt())
            && qFromLittleEndian<quint64>(header.constData()+8)==quint64(entry["chunk"].toInt())
            && s==unsigned(entry["source"].toInt()) && r==unsigned(entry["request"].toInt())
            && QCryptographicHash::hash(body,QCryptographicHash::Sha256).toHex()==entry["sha256"].toString().toLatin1(),"Source fixture exact body/identity");
        if(index==1){firstHeader=header;firstPacked=packed;}
    }
    check(index==unsigned(expected.size()) && file.size()<qint64(decoded/4),"Source fixture compression and complete order");
    unsigned refused=0;
    for(int mutation=0;mutation<6;++mutation){
        auto header=firstHeader,packed=firstPacked;
        if(mutation==0)header[28]=1;
        if(mutation==1)header[32]=2;
        if(mutation==2)header[40]=1;
        if(mutation==3)header[48]=char(uchar(header[48])^1);
        if(mutation==4){packed.append('x');qToLittleEndian<quint32>(quint32(packed.size()),header.data()+24);}
        if(mutation==5)packed[packed.size()/2]=char(uchar(packed[packed.size()/2])^1);
        bool rejected=false;try{Decoder invalid;invalid.decode(header,packed,qFromLittleEndian<quint32>(firstHeader.constData()+16)+qFromLittleEndian<quint32>(firstHeader.constData()+20));}catch(const std::runtime_error&){rejected=true;}
        check(rejected,"Malformed V4 source admitted");++refused;
    }
    QFile fixture(root+"/expected-index.json");const auto bytes=QJsonDocument(expected).toJson();check(fixture.open(QIODevice::WriteOnly|QIODevice::NewOnly) && fixture.write(bytes)==bytes.size(),"Expected-index interchange");
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"report",report},{"records",int(index)},
        {"decoded_bytes",QString::number(decoded)},{"stored_bytes",double(file.size())},{"exact_source_request_bytes",true},{"malformed_refused",int(refused)}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
