#include "../renderer/gpu_production_queue.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include "../compat/legacy/world_rolling_handoff.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <chrono>
#include <iostream>
#include <thread>
using namespace mnm;
static void require(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
static std::uint64_t word(const QByteArray& bytes,int offset,int length){std::uint64_t value=0;for(int i=0;i<length;++i)value|=std::uint64_t(std::uint8_t(bytes[offset+i]))<<(i*8);return value;}
static std::vector<std::uint8_t> read(QFile& file,std::uint64_t size){auto bytes=file.read(qint64(size));require(std::uint64_t(bytes.size())==size,"Truncated World fixture");return {bytes.begin(),bytes.end()};}
int main(int argc,char** argv)try{
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QGuiApplication app(argc,argv);require(argc==3,"Usage: async-world-replay-test PREFIX ASSETS");
    QFile file(argv[1]);require(file.open(QIODevice::ReadOnly)&&file.read(8)=="MNMPROF1","World fixture magic");
    auto opened=assets::AssetStore::create(argv[2]);if(auto* error=std::get_if<assets::Error>(&opened))throw std::runtime_error(error->detail);auto store=std::get<assets::AssetStore>(std::move(opened));
    QOpenGLContext root;root.setFormat(format);require(root.create(),"World share context");
    QOffscreenSurface producer,output;producer.setFormat(root.format());output.setFormat(root.format());producer.create();output.create();require(producer.isValid()&&output.isValid(),"World platform targets");
    render::GlBlitter client(&root,&output),baseline;std::unique_ptr<legacy::WorldRollingSession> native;
    legacy::WorldRollingSession control(baseline,store,false,true,true,true);
    const auto destination=client.allocate(800,600,{16,{0xf800,0x7e0,31}});
    render::GpuProductionQueue queue(&root,&producer,[&]{native.reset();});
    std::uint64_t generation=0,compared=0;
    while(!file.atEnd()){
        const auto header=file.read(16);require(header.size()==16,"World record header");auto g=word(header,0,8),sourceSize=word(header,8,4),requestSize=word(header,12,4);
        require(g==generation+1&&g<=512&&sourceSize>=16&&sourceSize<=16777216&&requestSize>=64&&requestSize<=197184,"World fixture bounds/order");
        const auto bytes=read(file,sourceSize),request=read(file,requestSize);auto decoded=legacy::decodeCanvasProducers(bytes,false);auto handoff=legacy::prepareRollingHandoff(decoded,0,request,1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
        auto stream=std::make_shared<legacy::CanvasProducerStream>(std::move(handoff.stream));
        control.consumeResident(g,*stream);const auto expected=control.diagnosticReadResident(baseline);
        require(queue.submit(g,[&,g,stream](render::GlBlitter& renderer){if(!native)native=std::make_unique<legacy::WorldRollingSession>(renderer,store,false,true,true,true);auto frame=native->consumeResident(g,*stream);require(native->completed()==g&&!native->worldReadbacks()&&!renderer.stats().nativeReadbacks&&!renderer.stats().rgbaReadbacks,"World producer readback/order");return frame;}),"World admission refused");
        render::GpuProductionQueue::Completion result;const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);
        while(!queue.take(result)){require(std::chrono::steady_clock::now()<deadline,"World completion deadline");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        require(result.sequence==g,"World completion gap");client.importGpu565(destination,result.frame);const auto actual=client.read(destination);
        require(actual.width==expected.width&&actual.height==expected.height&&actual.pixels==expected.pixels,"Cross-owner completed World pixels differ");
        ++compared;generation=g;
    }
    require(compared>=64,"World fixture needs a contiguous64+ prefix");queue.join();const auto stats=queue.stats();require(stats.stopped&&stats.error.empty()&&stats.submitted==compared&&stats.produced==compared&&stats.taken==compared&&!stats.outstanding&&stats.allocations<=2,"World producer teardown/counts");
    client.destroy(destination);require(!client.stats().surfaces,"World import surface leaked");
    std::cout<<QJsonDocument(QJsonObject{{"success",true},{"worlds",double(compared)},{"full_native_word_comparisons",double(compared)},{"ordinary_producer_readbacks",0},{"diagnostic_import_readbacks",double(compared)},{"producer_joined",true},{"immutable_allocations",double(stats.allocations)},{"renderer",QString::fromStdString(client.driver().renderer)}}).toJson(QJsonDocument::Compact).toStdString()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
