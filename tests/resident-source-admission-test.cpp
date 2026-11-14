#include "resident_world_session.hpp"
#include "../tests/resource-fixtures.hpp"
#include <QApplication>
#include <QJsonDocument>
#include <QFile>
#include <iostream>
using namespace mnm;
using namespace resource_test;
namespace mnm::legacy {
// Seed at the verified-source FIFO boundary. Wire/ACK/return verification is
// exercised separately by world-snapshot-queue-test; no original game is run.
struct WorldSnapshotQueueTestAccess {
 static void push(WorldSnapshotQueue& q,OwnedWorldSnapshot s){std::lock_guard<std::mutex> lock(q.mutex_);q.ready_.push_back(std::move(s));++q.state_.accepted;++q.state_.returned;++q.state_.outstanding;}
};
}
struct ResidentWorldSessionTestAccess {
 static void enqueue(ResidentWorldSession& s,legacy::OwnedWorldSnapshot frame){legacy::WorldSnapshotQueueTestAccess::push(*s.snapshotQueue_,std::move(frame));}
 static void future(ResidentWorldSession& s){s.nextFrame_=std::chrono::steady_clock::now()+std::chrono::hours(1);}
 static auto stats(ResidentWorldSession& s){return s.snapshotQueue_->stats();}
};
static legacy::OwnedWorldSnapshot frame(unsigned generation,const QByteArray& sprite){
 legacy::OwnedWorldSnapshot result;result.generation=generation;auto& ops=result.handoff.stream.operations;result.handoff.stream.version=3;result.handoff.stream.queues=1;
 const auto add=[&](unsigned op){legacy::CanvasProducer p{};p.fields[0]=96;p.fields[1]=ops.size()+1;p.fields[2]=op;p.fields[3]=7;p.fields[5]=5;p.fields[6]=3;p.fields[7]=5;ops.push_back(p);};
 if(generation==1){add(1);add(5);ops.back().fields[12]=5;ops.back().fields[13]=3;ops.back().fields[14]=0x1234;}
 else{add(5);ops.back().fields[12]=5;ops.back().fields[13]=3;ops.back().fields[14]=0x4321;}
 add(11);ops.back().fields[14]=1;
 // Authored SPR bytes follow the same native owned-raster fixture as the
 // resident renderer tests; no original game or original pixels are inputs.
 auto get=[&](unsigned at){const auto* p=reinterpret_cast<const unsigned char*>(sprite.data()+at);return unsigned(p[0])|(unsigned(p[1])<<8)|(unsigned(p[2])<<16)|(unsigned(p[3])<<24);};
 const auto table=24+get(16)*768,base=table+get(12)*4,at=base+get(table),size=get(at);
 add(9);auto& raster=ops.back();raster.fields[8]=2;raster.fields[12]=5;raster.fields[13]=3;raster.fields[15]=8;raster.fields[19]=size;raster.fields[18]=size;raster.fields[0]=96+size;
 raster.payload.assign(sprite.begin()+at,sprite.begin()+at+size);std::fill(raster.payload.begin()+28,raster.payload.begin()+32,0);
 add(10);ops.back().fields[14]=1;ops.back().fields[15]=4;
 add(12);ops.back().fields[14]=1;
 return result;
}
int main(int argc,char** argv)try{
 QApplication app(argc,argv);qunsetenv("MNM_ASYNC_WORLD_GPU");qunsetenv("MNM_DISPLAY_WHILE_PENDING");qputenv("MNM_WORLD_IMAGE_REUSE","1");
 QTemporaryDir temp;require(temp.isValid(),"Fixture directory");const auto root=std::filesystem::path(temp.path().toStdString());populate(root);
 NativeRenderTarget target{[]{return true;},[]{return static_cast<QOpenGLContext*>(nullptr);},[](auto){},[]{return QString();},nullptr,nullptr};
 ResidentWorldSession session(target,temp.filePath("world"),temp.path(),true);ResidentWorldSessionTestAccess::future(session);
 QFile body(temp.filePath("body.spr"));require(body.open(QIODevice::ReadOnly),"Authored sprite");const auto sprite=body.readAll();
 ResidentWorldSessionTestAccess::enqueue(session,frame(1,sprite));ResidentWorldSessionTestAccess::enqueue(session,frame(2,sprite));
 session.pollOwned(false);require(session.report()["completed"].toString()=="1","Verified source waited for display deadline");
 auto state=session.displayState();require(state.pending&&state.deadline>std::chrono::steady_clock::now(),"Import or display deadline lost");
 session.pollOwned(false);require(session.report()["completed"].toString()=="1","Pending import allowed next source");
 const auto nonce=session.report()["nonce"].toString().toULongLong();
 // The channel nonce is retained by the source-facing wire owner.
 QFile channel(temp.filePath("world"));require(channel.open(QIODevice::ReadOnly),"Read channel identity");const auto header=channel.read(32);std::uint64_t identity=0;for(int i=0;i<8;++i)identity|=std::uint64_t(std::uint8_t(header[24+i]))<<(8*i);
 (void)nonce;rejected([&]{session.import(2,identity);});rejected([&]{session.import(1,identity^1);});
 auto first=session.import(1,identity);require(first.valid(),"Native frame missing");
 session.pollOwned(false);require(session.report()["completed"].toString()=="2","Second source waited for display deadline");
 auto second=session.import(2,identity);require(second.valid(),"Second native frame missing");
 const auto q=ResidentWorldSessionTestAccess::stats(session);require(q.accepted==2&&q.returned==2&&q.imports==2&&!q.outstanding,"FIFO credits changed");
 require(!session.drawIndependent(2).valid(),"Supplemental replay ignored its future deadline");
 require(session.report()["world_readbacks"].toInt()==0 && session.report()["total_native_readbacks"].toInt()==0,"Ordinary source admission read pixels");
 std::cout<<"{\"success\":true,\"sources_before_display_deadline\":2,\"ordered_imports\":2,\"pending_backpressure\":true,\"identity_refusals\":2,\"replay_clock_preserved\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
