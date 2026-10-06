#include "commands.hpp"
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
static void require(bool ok,const char* text){if(!ok)throw std::runtime_error(text);}
static SurfaceCommand create(unsigned sequence,unsigned id,unsigned version=3){
    SurfaceCommand c;c.version=version;c.operation=1;c.sequence=sequence;c.words={id,8,6,16,0xf800,0x7e0,31};
    c.format={16,{0xf800,0x7e0,31}};c.image={8,6,std::vector<std::uint32_t>(48,id)};return c;
}
static unsigned policies(GlBlitter& gl){
    unsigned tests=0;const auto external=gl.create({1,1,{123}},{16,{0xf800,0x7e0,31}});
    const auto check=[&](bool ok,const char* reason){require(ok,reason);++tests;};
    const auto bad=[&](auto action,unsigned version=3){
        CommandConsumer c(gl,[](GpuFrame){}, {CommandDiagnostics::Verify,false});
        c.submit(create(1,1,version));c.submit(create(2,2,version));bool failed=false;
        try{action(c);}catch(const std::runtime_error&){failed=true;}
        check(failed && c.state()==CommandConsumerState::Failed,"Invalid command did not fail");
        check(c.result().liveSurfaces==0 && gl.stats().surfaces==1,"Failure ownership cleanup");
        check(gl.read(external).pixels[0]==123,"Unowned surface affected");
    };
    bad([](auto& c){SurfaceCommand q;q.version=3;q.operation=16;q.sequence=3;q.words={2,1,1};c.submit(q);});
    bad([](auto& c){SurfaceCommand q;q.version=3;q.operation=16;q.sequence=3;q.words={2,1,2};q.regions={{0,0,3,3},{2,2,4,4}};c.submit(q);});
    bad([](auto& c){SurfaceCommand q;q.version=3;q.operation=18;q.sequence=3;q.words={2,0};c.submit(q);});
    for(unsigned v:{1u,2u})bad([v](auto& c){SurfaceCommand q;q.version=v;q.operation=16;q.sequence=3;q.words={2,0,0};c.submit(q);},v);
    bad([](auto& c){SurfaceCommand q;q.version=3;q.operation=17;q.sequence=3;q.words={1,2,0,0,0,0,0,8,6,0,0,8,6};c.submit(q);
        q.operation=18;q.sequence=4;q.words={3,surfaceStatus::invalidRect};c.submit(q);});
    {
        CommandConsumer c(gl,[](GpuFrame){}, {CommandDiagnostics::Skip,false});c.submit(create(1,1));c.submit(create(2,2));
        SurfaceCommand q;q.version=3;q.operation=17;q.sequence=3;q.words={1,2,0,0,0,0,0,8,6,0,0,8,6};c.submit(q);
        q.operation=18;q.sequence=4;q.words={3,0xdeadbeef};c.submit(q);
        check(c.state()==CommandConsumerState::Active && c.result().skippedResultChecks==1,"Skip diagnostics changed admission");c.abort();
        check(c.result().liveSurfaces==0 && gl.stats().surfaces==1,"Abort ownership cleanup");
    }
    gl.destroy(external);check(gl.stats().surfaces==0,"Policy resources leaked");return tests;
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    try {
        require(argc==2,"Expected v3 stream fixture");QFile f(QString::fromLocal8Bit(argv[1]));require(f.open(QIODevice::ReadOnly),"Fixture open");const auto data=f.readAll();
        GlBlitter gl;GpuFrame latest;unsigned frames=0;
        CommandDecoder decoder(CommandStreamMode::Streaming);
        CommandConsumer consumer(gl,[&](GpuFrame frame){require(frame.valid(),"Empty GPU presentation");latest=frame;++frames;},
            {CommandDiagnostics::Skip,false,CommandStreamMode::Streaming});
        qsizetype offset=0;unsigned fragments=0;const unsigned sizes[]={1,3,11,17,53,4093,65536};
        while(offset<data.size()){
            const auto chunk=data.mid(offset,sizes[fragments%7]);offset+=chunk.size();++fragments;
            const auto commands=decoder.append(chunk);consumer.submit(commands.data(),commands.size());
        }
        decoder.finish();consumer.finish();const auto result=consumer.result();
        require(consumer.state()==CommandConsumerState::Ended && result.liveSurfaces==0 && result.livePalettes==0,"Fragmented replay incomplete");
        require(result.surfaceCopies==408 && result.surfaceCopyFailures==312 && result.skippedResultChecks==408 && result.skippedChecks==816,"Fragmented replay counts");
        require(result.stats.nativeReadbacks==0 && result.stats.rgbaReadbacks==0 && result.stats.gpuPresentations==1 && frames==1,"Ordinary replay readbacks/presentation");
        const auto checks=policies(gl);
        QJsonObject report{{"success",true},{"fragments",int(fragments)},{"surface_copies",int(result.surfaceCopies)},
            {"surface_failures",int(result.surfaceCopyFailures)},{"skipped_result_checks",int(result.skippedResultChecks)},
            {"ordinary_readbacks",0},{"frames",int(frames)},{"live_surfaces",0},{"policy_checks",int(checks)}};
        std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
}
