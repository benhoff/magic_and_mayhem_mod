#include "../apps/qt-shell/resident_profile_capture.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QByteArray>
#include <iostream>
#include <stdexcept>
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);if(argc!=2)return 2;const QString root=argv[1];require(QDir().mkpath(root),"Capture fixture directory");
    std::vector<std::uint8_t> source(4096),request(64,9);QByteArray expected("MNMPROF1",8);
    ResidentProfileCapture normal(root+"/worker.bin");
    for(unsigned seq=1;seq<=4;++seq){
        for(unsigned i=0;i<source.size();++i)source[i]=std::uint8_t(i*17+seq);
        require(normal.enqueue(seq,source,request),"Valid capture enqueue");
        QByteArray header(16,0);for(unsigned i=0;i<8;++i)header[int(i)]=char(std::uint64_t(seq)>>(i*8));
        for(unsigned i=0;i<4;++i){header[int(8+i)]=char(unsigned(source.size())>>(i*8));header[int(12+i)]=char(unsigned(request.size())>>(i*8));}
        expected+=header;expected.append(reinterpret_cast<const char*>(source.data()),qsizetype(source.size()));expected.append(reinterpret_cast<const char*>(request.data()),qsizetype(request.size()));
    }
    normal.finish();normal.finish();const auto complete=normal.report();
    require(complete["complete"].toBool() && complete["written_worlds"].toInt()==4 && complete["queue_bytes"].toInt()==0,"Capture worker retirement");
    QFile file(root+"/expected-v1.bin");require(file.open(QIODevice::WriteOnly|QIODevice::NewOnly) && file.write(expected)==expected.size(),"Expected capture bytes");file.close();
    ResidentProfileCapture history(root+"/sources.bin",256ull*1024*1024,true);
    require(history.enqueueSource(1,source)&&history.enqueueSource(1,source)&&history.enqueue(1,source,request),"Source prefixes before first World");
    require(history.enqueueSource(2,source)&&history.enqueue(2,source,request),"Next source prefix and World");
    history.finish();require(history.report()["complete"].toBool()&&history.report()["written_worlds"].toInt()==2&&history.report()["written_source_chunks"].toInt()==3,"Prefix capture ownership/counters");
    require(!history.enqueueSource(3,source),"Retired source capture resumed");
    ResidentProfileCapture legacySource(root+"/legacy-source.bin");require(!legacySource.enqueueSource(1,source),"Legacy capture silently accepts source-only input");legacySource.finish();require(!legacySource.report()["complete"].toBool(),"Legacy source refusal lost");
    ResidentProfileCapture sourceOrder(root+"/source-order.bin",256ull*1024*1024,true);require(!sourceOrder.enqueueSource(2,source),"Source skips World generation");sourceOrder.finish();
    ResidentProfileCapture capped(root+"/source-cap.bin",256ull*1024*1024,true);
    const std::vector<std::uint8_t> tiny(16,7);
    for(unsigned i=0;i<8192;++i)require(capped.enqueueSource(1,tiny),"Bounded prefix history prematurely refused");
    require(!capped.enqueue(1,tiny,request),"World bypassed total source-record cap");capped.finish();require(capped.report()["truncated"].toBool()&&!capped.report()["complete"].toBool(),"Source-record cap not retained");
    ResidentProfileCapture overflow(root+"/overflow.bin",128);require(!overflow.enqueue(1,source,request),"Queue overflow admitted");require(!overflow.enqueue(1,source,request),"Truncated capture resumed");overflow.finish();require(overflow.report()["truncated"].toBool() && !overflow.report()["complete"].toBool(),"Overflow not retained");
    ResidentProfileCapture order(root+"/order.bin");require(!order.enqueue(2,source,request),"Generation gap admitted");order.finish();require(!order.report()["error"].toString().isEmpty(),"Generation error missing");
    ResidentProfileCapture extent(root+"/extent.bin");require(!extent.enqueue(1,source,{1}),"Short request admitted");extent.finish();require(!extent.report()["complete"].toBool(),"Invalid extent completed");
    const auto occupied=root+"/occupied.bin";QFile sentinel(occupied);require(sentinel.open(QIODevice::WriteOnly|QIODevice::NewOnly) && sentinel.write("preserved",9)==9,"Sentinel");sentinel.close();
    ResidentProfileCapture exists(occupied);exists.finish();require(!exists.report()["error"].toString().isEmpty(),"Existing file admitted");require(sentinel.open(QIODevice::ReadOnly) && sentinel.readAll()=="preserved","Existing file overwritten");
    bool refused=false;try{ResidentProfileCapture invalid(root+"/invalid.bin",127);}catch(const std::runtime_error&){refused=true;}require(refused,"Queue policy admitted invalid bound");
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"normal",complete},{"overflow",overflow.report()},{"controls",5},{"exact_raw_bytes",expected.size()}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
