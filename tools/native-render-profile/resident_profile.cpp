#include "../../compat/legacy/world_rolling_session.hpp"
#include "../../compat/legacy/world_rolling_handoff.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QCryptographicHash>
#include <QThread>
#include <algorithm>
#include <iostream>
using namespace mnm;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::uint64_t word(const QByteArray& bytes,int offset,int length){
    std::uint64_t value=0;for(int i=0;i<length;++i)value|=std::uint64_t(std::uint8_t(bytes[offset+i]))<<(i*8);return value;
}
static std::vector<std::uint8_t> read(QFile& file,std::uint64_t size){
    const auto bytes=file.read(qint64(size));require(std::uint64_t(bytes.size())==size,"Truncated resident profile corpus");return {bytes.begin(),bytes.end()};
}
int main(int argc,char** argv) try {
    QGuiApplication app(argc,argv);
    if(argc!=6){std::cerr<<"Usage: mnm-resident-profile CORPUS ASSETS {timestamps|throughput} REPEATS REPORT\n";return 2;}
    require(qgetenv("DISPLAY").isEmpty() && qgetenv("WAYLAND_DISPLAY").isEmpty(),"Resident benchmark must have no desktop display");
    const bool timed=QByteArray(argv[3])=="timestamps";require(timed||QByteArray(argv[3])=="throughput","Invalid resident benchmark mode");
    const int repeats=QByteArray(argv[4]).toInt();require(repeats>=1 && repeats<=4,"Resident repeats outside1..4");
    auto opened=assets::AssetStore::create(argv[2]);if(const auto* error=std::get_if<assets::Error>(&opened))throw std::runtime_error(error->detail);
    const auto store=std::get<assets::AssetStore>(std::move(opened));
    QJsonArray runs;QString expectedDigest,driverName;std::uint64_t expectedWorlds=0;
    for(int repeat=0;repeat<repeats;++repeat){
        QFile file(argv[1]);require(file.open(QIODevice::ReadOnly) && file.size()>=8 && file.size()<=16ll*1024*1024*1024,"Unbounded/unavailable resident corpus");
        require(file.read(8)=="MNMPROF1","Resident profile capture magic");
        QElapsedTimer clock;clock.start();
        render::GlBlitter renderer;const auto driver=renderer.driver();driverName=QString::fromStdString(driver.renderer);
        require(driver.vendor.find("NVIDIA")!=std::string::npos && driver.renderer.find("TITAN")!=std::string::npos,"Expected NVIDIA TITAN; refusing fallback");
        const auto ms=[&]{return double(clock.nsecsElapsed())/1000000.;};
        QJsonArray rows;std::uint64_t sequence=0,dropped=0;double completionMs=0;QString digest;
        {
            legacy::WorldRollingSession session(renderer,store,false,true);render::GpuFrame frame;
            const auto collect=[&]{for(const auto& timing:renderer.pollGpuTimings()){
                if(timing.sequence==8193)continue; // end-of-stream completion marker
                require(timing.sequence<=std::uint64_t(rows.size()),"Benchmark GPU timing identity");
                auto row=rows[int(timing.sequence-1)].toObject();row["gpu_timeline_ms"]=timing.timelineMs;rows[int(timing.sequence-1)]=row;
            }};
            while(!file.atEnd()){
                const double ioStart=ms();const auto header=file.read(16);require(header.size()==16,"Truncated resident record header");
                const auto generation=word(header,0,8),sourceSize=word(header,8,4),requestSize=word(header,12,4);
                require(generation==sequence+1 && generation<=8192 && sourceSize>=16 && sourceSize<=16*1024*1024 && requestSize>=64 && requestSize<=197184,"Invalid resident record sequence/extent");
                const auto sourceBytes=read(file,sourceSize),requestBytes=read(file,requestSize);const double ioEnd=ms();
                const auto decoded=legacy::decodeCanvasProducers(sourceBytes,false);const double decodeEnd=ms();
                const auto handoff=legacy::prepareRollingHandoff(decoded,0,requestBytes,1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);const double handoffEnd=ms();
                ++sequence;
                if(timed){if(!renderer.profileBatch(sequence,[&]{frame=session.consumeResident(sequence,handoff.stream);}))++dropped;}
                else frame=session.consumeResident(sequence,handoff.stream);
                const double consumeEnd=ms();
                require(frame.valid() && session.completed()==sequence && !session.worldReadbacks() && !renderer.stats().nativeReadbacks && !renderer.stats().rgbaReadbacks,"Resident delivery/readback/completion contract");
                rows.append(QJsonObject{{"generation",QString::number(sequence)},{"source_read_ms",ioEnd-ioStart},{"decode_ms",decodeEnd-ioEnd},
                    {"handoff_ms",handoffEnd-decodeEnd},{"native_consume_ms",consumeEnd-handoffEnd},
                    {"native_cpu_producer_ms",session.producerWorkMs()},{"native_prepare_ms",session.profile().prepareMs},
                    {"native_submit_cpu_ms",session.profile().submitMs},{"world_rasters",int(handoff.batch.header[14])},
                    {"source_bytes",double(sourceSize)},{"uploads",double(renderer.stats().uploads)},
                    {"copies",double(renderer.stats().copies)},{"copy_batches",double(renderer.stats().copyBatches)}});
                if(timed)collect();
            }
            require(sequence>=64,"Resident benchmark needs at least64 actual ordered Worlds");
            // A terminal empty timestamp marker completes prior GPU commands.
            // No glFinish, synchronous per-frame query reads or pixel return.
            collect();require(renderer.profileBatch(8193,[]{}),"Terminal completion marker capacity");
            QElapsedTimer deadline;deadline.start();
            while(renderer.pendingGpuTimings()){
                collect();require(deadline.elapsed()<30000,"Offline GPU completion deadline");
                if(renderer.pendingGpuTimings())QThread::msleep(1);
            }
            completionMs=ms();
            // One diagnostic readback after the whole replay completion timer.
            const auto image=session.diagnosticReadResident(renderer);
            QByteArray words;words.resize(qsizetype(image.pixels.size()*4));
            for(std::size_t i=0;i<image.pixels.size();++i)for(unsigned k=0;k<4;++k)words[qsizetype(i*4+k)]=char(image.pixels[i]>>(k*8));
            digest=QString::fromLatin1(QCryptographicHash::hash(words,QCryptographicHash::Sha256).toHex());frame={};
        }
        require(!renderer.stats().surfaces && !renderer.pendingGpuTimings() && !dropped,"Resident benchmark resource/query cleanup");
        if(!repeat){expectedDigest=digest;expectedWorlds=sequence;}else require(digest==expectedDigest && sequence==expectedWorlds,"Repeated resident replay changed output/count");
        runs.append(QJsonObject{{"repeat",repeat},{"worlds",double(sequence)},{"completion_ms",completionMs},{"completion_worlds_per_second",double(sequence)*1000/completionMs},
            {"samples",rows},{"final_native_words_sha256",digest},{"world_readbacks",0},{"delivery_readbacks",0},{"diagnostic_readbacks_outside_timing",1},
            {"query_drops",double(dropped)},{"remaining_queries",0},{"remaining_surfaces",0}});
    }
    QJsonObject report{{"success",true},{"renderer",driverName},{"mode",timed?"timestamps":"throughput"},{"runs",runs},
        {"scope","Actual captured public World sequences through unchanged decode/ownedV5 handoff/resident consumption, ordered cold-to-complete replay on NVIDIA TITAN EGL. All-frame completion throughput includes disk acquisition, decode, handoff, native CPU, GPU drain and cold startup; one diagnostic hash follows timing. No original engine pacing/simulation, command import/UI, Qt widget paint or scanout. Timestamp GPU intervals include CPU submission starvation; timestamp-disabled mode has only an end-of-stream completion marker."}};
    QFile output(argv[5]);require(output.open(QIODevice::WriteOnly|QIODevice::NewOnly) && output.write(QJsonDocument(report).toJson())>0,"Cannot retain resident benchmark report");
    std::cout<<expectedWorlds<<" actual ordered Worlds, "<<repeats<<" completed replays on "<<driverName.toStdString()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
