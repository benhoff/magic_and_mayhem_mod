#include "../renderer/failure_diagnostics.hpp"
#include "../renderer/blit.hpp"
#include "../renderer/commands.hpp"
#include "../apps/qt-shell/native_only_session.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QTemporaryDir>
#include <cassert>
#include <iostream>
#include <thread>
using namespace mnm::render;
static QJsonObject call(const QJsonObject& report,const char* name){
    for(const auto& v:report["active"].toArray())if(v.toObject()["call"]==name)return v.toObject();
    for(const auto& v:report["recent"].toArray())if(v.toObject()["call"]==name)return v.toObject();
    return {};
}
static QString undefinedRead(GlBlitter& renderer,SurfaceId source){
    try{renderer.presentGpu(source);}catch(const std::exception& e){return QString::fromUtf8(e.what());}
    throw std::runtime_error("Undefined pixels were accepted");
}
int main(int argc,char** argv)try{
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QGuiApplication app(argc,argv);
    DiagnosticCall::reset();
    for(unsigned i=0;i<1000;++i){DiagnosticCall history("old_call",{i});}
    auto bounded=DiagnosticCall::snapshot();assert(bounded["recent"].toArray().size()==64);
    assert(bounded["recent"].toArray().first().toObject()["serial"]=="937");
    QString error;
    const QByteArray payload("independent input bytes");
    const PixelFormat rgb{16,{0xf800,0x7e0,31}};
    GlBlitter renderer;
    const auto source=renderer.allocate(4,3,rgb);renderer.update(source,0,0,{2,1,{31,0x7e0}});
    {
        DiagnosticCall generation("world_generation",{46844,46843,3});
        DiagnosticCall original("world_record");std::array<std::uint32_t,24> words{};
        words[1]=1940;words[2]=10;words[3]=47;words[22]=0x5003b9;original.append(words);original.payload(payload.data(),payload.size());
        // Keep active provenance even when the bounded recent ring wraps.
        for(unsigned i=0;i<80;++i){DiagnosticCall history("intervening",{i});}
        error=undefinedRead(renderer,source);
    }
    const auto report=renderFailureDetails(error),detail=report["detail"].toObject();
    assert(report["executable_hash_available"].toBool()&&report["executable_sha256"].toString().size()==64);
    assert(detail["operation"]=="presentGpu"&&detail["file"]=="renderer/blit.cpp"&&detail["line"].toInt()>0);
    assert(detail["surface"].toObject()["id"]==QString::number(source));
    assert(detail["unknown_pixels"].toInt()==10&&detail["first_unknown"]==QJsonArray({2,0}));
    assert(detail["read_rect"]==QJsonArray({0,0,4,3}));
    assert(detail["surface"].toObject()["invalidations"]=="1"&&detail["surface"].toObject()["definitions"]=="1");
    assert(detail["live_surfaces"].toArray().size()==1&&detail["validity_sha256"].toString().size()==64);
    assert(call(report,"world_record")["words"].toArray()[22]==QString::number(0x5003b9));
    assert(call(report,"world_record")["payload_sha256"]==QString::fromLatin1(QCryptographicHash::hash(payload,QCryptographicHash::Sha256).toHex()));
    assert(call(report,"world_record")["payload_prefix_hex"]==QString::fromLatin1(payload.toHex()));
    assert(call(report,"world_generation")["words"].toArray().first()=="46844");
    assert(!renderer.stats().nativeReadbacks&&!renderer.stats().rgbaReadbacks);
    renderer.destroy(source);
    for(unsigned i=0;i<80;++i){DiagnosticCall cleanup("cleanup",{i});}
    assert(renderFailureDetails("string adapter: "+error)==report);
    assert(renderFailureDetails(QString::fromUtf8(renderFailure(error.toUtf8().constData()).what()))==report);
    std::thread forwarded([&]{error="GPU thread forwarding: "+error;});forwarded.join();
    QTemporaryDir directory;assert(directory.isValid());NativeOnlySession session;assert(session.begin(directory.path()));
    assert(session.fail(error)&&session.fail("later teardown error"));
    QFile retained(directory.filePath("native-only-session.json"));assert(retained.open(QIODevice::ReadOnly));
    assert(QJsonDocument::fromJson(retained.readAll()).object()["render_failure"]==report);
    assert(renderFailureSummary(error).contains("Surface pixels are undefined"));
    assert(renderFailureDetails("ordinary failure").isEmpty());
    assert(renderFailureDetails(QString(failureMarker)+"AAAAAA==").isEmpty());
    assert(renderFailureDetails(QString(failureMarker)+"f////w==").isEmpty());
    // Generic UI rejection is snapshotted before consumer cleanup overwrites history.
    DiagnosticCall::reset();
    CommandConsumer consumer(renderer,[](auto){},{CommandDiagnostics::Skip,false,CommandStreamMode::Streaming});
    SurfaceCommand command;command.version=3;command.sequence=1;command.operation=6;command.words[0]=999;
    try{consumer.submit(command);assert(false);}catch(const std::exception& e){
        const auto failure=renderFailureDetails(QString::fromUtf8(e.what()));
        const auto ui=call(failure,"ui_command");assert(ui["words"].toArray()==QJsonArray({"3","1","6","999","0","0","0","0","0","0","0","0","0","0","0","0"}));
        assert(ui["failed"].toBool());
    }
    // Actual source-only adapter failure retains original caller/payload fields.
    auto store=mnm::assets::AssetStore::create(directory.path().toStdString());assert(std::holds_alternative<mnm::assets::AssetStore>(store));
    mnm::legacy::WorldRollingSession world(renderer,std::get<mnm::assets::AssetStore>(store),false,true);
    mnm::legacy::CanvasProducer c{};c.fields[1]=7;c.fields[2]=2;c.fields[3]=47;c.fields[22]=0x4a4c26;
    mnm::legacy::CanvasProducerStream stream{0,{c},3};
    try{world.consumeSource(stream);assert(false);}catch(const std::exception& e){
        const auto failure=renderFailureDetails(QString::fromUtf8(e.what()));
        assert(call(failure,"source_record")["words"].toArray()[22]==QString::number(0x4a4c26));
        assert(world.failed());
    }
    assert(!renderer.stats().surfaces);
    std::cout<<"{\"success\":true,\"history_bound\":64,\"undefined_read_metadata\":true,\"source_caller_retained\":true,\"ui_command_retained\":true,\"cleanup_survival\":true,\"thread_string_forwarding\":true,\"sticky_terminal_report\":true,\"pixel_readbacks\":0,\"original_game_runs\":0}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
