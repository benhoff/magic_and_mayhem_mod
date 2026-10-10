#include "../compat/legacy/world_rolling_session.hpp"
#include "../compat/legacy/world_rolling_channel.hpp"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QSurfaceFormat>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>
#include <random>
#include <limits>
using namespace mnm;
using Clock=std::chrono::steady_clock;
static void require(bool ok,const char* m){if(!ok)throw std::runtime_error(m);}
template<class F> static void reject(F action){try{action();}catch(const std::exception&){return;}throw std::runtime_error("Invalid rolling packet accepted");}
static QByteArray load(const QString& p){QFile f(p);require(f.open(QIODevice::ReadOnly),"Input file missing");return f.readAll();}
static double elapsed(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
static double rss(){std::ifstream f("/proc/self/statm");std::uint64_t total=0,resident=0;f>>total>>resident;require(bool(f),"RSS sample failed");return double(resident)*sysconf(_SC_PAGESIZE);}
static QString digest(const render::Image& image){QByteArray words;words.resize(image.pixels.size()*2);auto* bytes=words.data();for(std::size_t i=0;i<image.pixels.size();++i){bytes[2*i]=char(image.pixels[i]);bytes[2*i+1]=char(image.pixels[i]>>8);}return QString::fromLatin1(QCryptographicHash::hash(words,QCryptographicHash::Sha256).toHex());}
static void copySpans(){
  std::mt19937 random(20261010);render::CanvasSequence canvases;canvases.create(1,6,4);canvases.create(2,6,4);
  for(unsigned n=0;n<300;++n){render::Image source{6,4,std::vector<std::uint32_t>(24)},dest=source;for(auto& p:source.pixels)p=random()%8;for(auto& p:dest.pixels)p=random()%65536;
    const bool alias=n%2;canvases.update(1,0,0,source);canvases.update(2,0,0,dest);auto expected=alias?source:dest;const auto before=expected;
    const int left=random()%6,top=random()%4;const render::Rect r{left,top,left+int(random()%(7-left)),top+int(random()%(5-top))};
    const int x=n%31==0?std::numeric_limits<int>::min():n%37==0?std::numeric_limits<int>::max():int(random()%15)-7,y=n%41==0?std::numeric_limits<int>::min():int(random()%11)-5;
    const std::optional<std::uint16_t> key=n%3?std::nullopt:std::optional<std::uint16_t>(random()%8);
    for(int row=r.top;row<r.bottom;++row)for(int col=r.left;col<r.right;++col){const auto dx=std::int64_t(x)+col-r.left,dy=std::int64_t(y)+row-r.top;if(dx<0||dy<0||dx>=6||dy>=4)continue;const auto word=(alias?before:source).pixels[row*6+col];if(!key||word!=*key)expected.pixels[dy*6+dx]=word;}
    canvases.copy(1,alias?1:2,r,x,y,key);require(canvases.read(alias?1:2).pixels==expected.pixels,"Clipped/aliased/keyed copy differs from scalar snapshot oracle");if(!alias)require(canvases.read(1).pixels==source.pixels,"Distinct copy mutated its source");
  }
  canvases.create(3,6,4);const auto before=canvases.read(2);reject([&]{canvases.copy(3,2,{0,0,6,4},0,0);});require(canvases.read(2).pixels==before.pixels,"Undefined source partially mutated destination");
  canvases.fill(3,{0,0,1,1},7);reject([&]{canvases.copy(3,2,{0,0,2,1},0,0,7);});require(canvases.read(2).pixels==before.pixels,"Key skipped undefined source preflight");
  canvases.copy(3,2,{0,0,6,4},10,10);require(canvases.read(2).pixels==before.pixels,"Fully clipped copy read or wrote storage");
  canvases.copy(3,3,{0,0,1,1},1,0);reject([&]{canvases.read(3);});reject([&]{canvases.copy(3,2,{2,0,3,1},0,0);});
}
int main(int argc,char** argv){
  QSurfaceFormat f;f.setVersion(3,3);f.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(f);QGuiApplication app(argc,argv);QJsonObject report{{"success",false}};
  if(argc!=6){std::cerr<<"Usage: native-world-rolling INPUT ASSETS EXPECTED OUTPUT QUEUES\n";return 2;}
  try{
    copySpans();report["scalar_copy_cases"]=300;
    const unsigned target=std::stoul(argv[5]);require(target>=64&&target<=10000,"Require64..10000 rolling queues");
    const auto raw=load(argv[1]);const auto stream=legacy::decodeCanvasProducers({raw.begin(),raw.end()});const auto expected=QJsonDocument::fromJson(load(argv[3])).object()["checkpoints"].toArray();require(!expected.empty()&&stream.queues==32,"Saved corpus must be a closed32 prefix");
    auto opened=assets::AssetStore::create(argv[2]);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);const auto store=std::get<assets::AssetStore>(std::move(opened));
    std::vector<std::vector<legacy::CanvasProducer>> blocks;std::vector<legacy::CanvasProducer> block;
    for(const auto& c:stream.operations){block.push_back(c);if(c.fields[2]==12){blocks.push_back(std::move(block));block={};}}
    require(blocks.size()==32&&block.empty(),"Saved block split failed");
    std::vector<std::vector<std::uint8_t>> packets;std::vector<QString> finalHashes;unsigned savedChecks=0;
    for(const auto& b:blocks){packets.push_back(legacy::encodeRollingQueue(b));QString last;unsigned canvas=0;
      for(const auto& c:b){if(c.fields[2]==11)canvas=c.fields[3];if(c.fields[2]==10){require(savedChecks<unsigned(expected.size()),"Unexpected saved checkpoint");auto e=expected[int(savedChecks++)].toObject();require(unsigned(e["sequence"].toInt())==c.fields[1]&&unsigned(e["canvas"].toInt())==c.fields[3]&&unsigned(e["oracle"].toInt())==c.fields[14],"Saved checkpoint identity mismatch");if(c.fields[3]==canvas)last=e["sha256"].toString();}}
      require(!last.isEmpty(),"Missing final queue digest");finalHashes.push_back(last);
    }
    require(savedChecks==unsigned(expected.size()),"Missing saved checkpoints");
    auto moving=blocks.back();bool moved=false,entered=false;
    for(auto& c:moving){if(c.fields[2]==11)entered=true;if(entered&&c.fields[2]==9&&c.fields[15]==1&&c.fields[14]==0&&!moved){++c.fields[8];moved=true;}}
    require(moved,"No indexed copy draw for changed-input continuation");
    packets.push_back(legacy::encodeRollingQueue(moving));require(packets[32]!=packets[31],"Changed input produced the same packet");
    unsigned refused=0;for(unsigned at:{0u,8u,12u,16u,20u,24u,28u,60u,68u,72u,156u}){auto bad=packets.front();bad[at]^=128;reject([&]{legacy::decodeRollingQueue(bad);});++refused;}
    auto truncated=packets.back();truncated.pop_back();reject([&]{legacy::decodeRollingQueue(truncated);});++refused;
    auto doubleQueue=blocks.back();doubleQueue.insert(doubleQueue.end(),blocks.back().begin(),blocks.back().end());reject([&]{legacy::encodeRollingQueue(doubleQueue);});++refused;
    auto get=[](const auto& bytes,std::size_t at){return unsigned(bytes.at(at))|unsigned(bytes.at(at+1))<<8|unsigned(bytes.at(at+2))<<16|unsigned(bytes.at(at+3))<<24;};
    bool referenceChecked=false,ownershipChecked=false;
    auto mutablePacket=packets.front();auto owned=legacy::decodeRollingQueue(mutablePacket);
    for(std::size_t at=64;at<packets.front().size();at+=get(packets.front(),at)){
      if(get(packets.front(),at+8)==25&&!ownershipChecked){mutablePacket[at+96]^=1;const auto source=get(packets.front(),at+4);require(owned.ownedPayloads.at(source).bytes()[0]==packets.front()[at+96],"Decoded source borrowed mutable packet storage");ownershipChecked=true;}
      if((get(packets.front(),at+8)==8||get(packets.front(),at+8)==9)&&get(packets.front(),at+16)&&!referenceChecked){auto bad=packets.front();std::fill_n(bad.begin()+at+16,4,255);reject([&]{legacy::decodeRollingQueue(bad);});++refused;referenceChecked=true;}
    }
    require(referenceChecked&&ownershipChecked,"Packet-local ownership/refusal fixture did not execute");
    render::GlBlitter renderer;const auto driver=renderer.driver();report["driver"]=QJsonObject{{"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)},{"version",QString::fromStdString(driver.version)}};
    // A rejected sequence poisons the service; retries cannot present a partial
    // history. Recovery is a separate service/session, never a queue-number reset.
    {legacy::WorldRollingSession invalid(renderer,store);reject([&]{invalid.consume(2,packets.front());});require(invalid.failed(),"Sequence failure not terminal");reject([&]{invalid.consume(1,packets.front());});refused+=2;}
    const auto initialize=Clock::now();QTemporaryDir directory;require(directory.isValid(),"Channel directory");const auto path=directory.filePath("rolling.bin");const std::uint64_t epoch=0x2026101000000001ULL;legacy::WorldRollingChannel::create(path,epoch);
    std::atomic<bool> stop{false};std::atomic<unsigned> pressure{0},acks{0};std::string producerError;
    std::thread producer([&]{try{
      legacy::WorldRollingChannel p(path,epoch,legacy::WorldRollingChannel::Role::producer);unsigned sent=0,acked=0;auto progress=Clock::now();
      while(acked<target&&!stop){
        if(sent<target){const auto packet=sent>=32&&(sent+1)%64==0?32u:std::min(sent,31u);if(p.publish(packets[packet])){++sent;progress=Clock::now();if(sent%17==0)std::this_thread::sleep_for(std::chrono::milliseconds(1));}else ++pressure;}
        if(auto done=p.reap()){require(done->sequence==++acked,"ACK was skipped or reordered");if(acked<=32)require(digest(done->image)==finalHashes[acked-1],"Returned queue pixels differ from saved digest");acks=acked;progress=Clock::now();}
        require(elapsed(progress)<30000,"Producer ACK deadline exceeded");std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    }catch(const std::exception& e){producerError=e.what();stop=true;}});
    QJsonArray frames;unsigned checked=0;std::unique_ptr<legacy::WorldRollingChannel> consumer;
    try{
      consumer=std::make_unique<legacy::WorldRollingChannel>(path,epoch,legacy::WorldRollingChannel::Role::consumer);auto& c=*consumer;
      {legacy::WorldRollingSession session(renderer,store);report["initialization_ms"]=elapsed(initialize);std::this_thread::sleep_for(std::chrono::milliseconds(50));
        auto progress=Clock::now();
        while(session.completed()<target&&!stop){
          const auto pollStart=Clock::now();if(auto in=c.poll()){
            const auto start=pollStart;if(in->sequence%11==0)std::this_thread::sleep_for(std::chrono::milliseconds(3));
            std::function<void(const legacy::CanvasProducer&,const render::Image&)> verify;
            if(in->sequence<=32)verify=[&](const auto& op,const auto& pixels){if(in->sequence<=32){require(checked<unsigned(expected.size()),"Extra checkpoint");const auto e=expected[int(checked++)].toObject();require(unsigned(e["canvas"].toInt())==op.fields[3]&&unsigned(e["oracle"].toInt())==op.fields[14]&&digest(pixels)==e["sha256"].toString(),"Saved checkpoint pixels/identity changed");}};
            const auto image=session.consume(in->sequence,in->bytes,verify);
            const auto& p=session.profile();const auto cache=session.producerCacheStats();require(p.identityFrames<=4096&&p.identityBytes<=16*1024*1024&&p.cacheFrames<=4096&&p.cacheSurfaces<=64&&cache.frames<=4096&&cache.bytes<=64*1024*1024,"Retained cache exceeds bound");
            const auto packetCache=session.packetCacheStats();require(packetCache.bytes<=MNM_ROLL_INPUT&&packetCache.records<=131072&&packetCache.sources<=2048&&packetCache.sourceBytes<=16*1024*1024,"Retained packet cache exceeds bound");
            if(in->sequence>32)require(p.cacheUploads==0&&p.identityHashes==0&&p.visualChecks==0,"Continued unchanged requests repeated upload/hash");
            c.complete(in->sequence,image);const auto verifiedMs=elapsed(start);frames.append(QJsonObject{{"queue",double(in->sequence)},{"native_work_ms",session.nativeWorkMs()},{"cpu_producer_ms",session.nativeWorkMs()-p.adoptMs-p.prepareMs-p.submitMs-p.readbackMs-p.compareMs},{"gpu_submit_ms",p.submitMs},{"resource_prepare_ms",p.prepareMs},{"delivery_completion_ms",verifiedMs-session.checkpointAssertionMs()},{"verified_completion_ms",verifiedMs},{"checkpoint_assertion_ms",session.checkpointAssertionMs()},{"rss_bytes",rss()},{"packet_decodes",double(packetCache.decodes)},{"packet_hits",double(packetCache.hits)},{"packet_bytes",double(packetCache.bytes)},{"packet_sources",double(packetCache.sources)},{"uploads",double(p.cacheUploads)},{"identity_bytes",double(p.identityBytes)},{"cache_frames",double(p.cacheFrames)},{"cache_surfaces",double(p.cacheSurfaces)}});
            progress=Clock::now();
          }
          require(elapsed(progress)<30000,"Consumer input deadline exceeded");std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
        require(session.completed()==target&&!session.failed(),"Rolling service stopped early");require(session.packetCacheStats().hits>0&&session.packetCacheStats().decodes>32,"Packet cache did not exercise hits and changed-input invalidation");report["world_queues"]=double(session.completed());
      }
      auto deadline=Clock::now();while(acks<target&&!stop){require(elapsed(deadline)<30000,"Final ACK deadline exceeded");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
      require(!stop&&acks==target,"Producer failed or missing ACK");
    }catch(...){if(consumer)consumer->cancel();stop=true;producer.join();throw;}
    producer.join();require(producerError.empty(),producerError.c_str());
    require(pressure>0&&checked==unsigned(expected.size())&&renderer.stats().surfaces==0&&renderer.stats().pixels==0,"Missing pressure/checkpoints or leaked surfaces");
    report["frames"]=frames;report["source_packet_refusals"]=int(refused);report["packet_source_ownership_checked"]=ownershipChecked;report["first_packet_bytes"]=double(packets.front().size());report["continuation_packet_bytes"]=double(packets[31].size());report["changed_request_queues"]=int(target/64);report["saved_checkpoint_comparisons"]=int(checked);report["acknowledgements"]=int(acks);report["backpressure_attempts"]=int(pressure);report["synthetic_continuation_queues"]=int(target-32);report["remaining_surfaces"]=double(renderer.stats().surfaces);report["original_pixels_used_as_native_inputs"]=false;report["success"]=true;
  }catch(const std::exception& e){report["error"]=QString::fromLocal8Bit(e.what());std::cerr<<e.what()<<'\n';}
  QFile out(argv[4]);if(!out.open(QIODevice::WriteOnly|QIODevice::NewOnly)){std::cerr<<"Cannot create report\n";return 1;}out.write(QJsonDocument(report).toJson());return report["success"].toBool()?0:1;
}
