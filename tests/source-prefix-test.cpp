#include "../compat/legacy/world_continuous.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include "../protocols/include/mnm/world_continuous_checksum.h"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <cstring>
#include <iostream>
using namespace mnm;
using namespace mnm::legacy;
using namespace resource_test;
namespace {
CanvasProducer operation(unsigned kind,unsigned id=7) {
 CanvasProducer c{};c.fields[2]=kind;c.fields[3]=id;c.fields[5]=5;c.fields[6]=3;
 c.fields[12]=5;c.fields[13]=3;return c;
}
std::vector<std::uint8_t> prefix(const std::vector<CanvasProducer>& operations) {
 std::vector<std::uint8_t> bytes(64);std::memcpy(bytes.data(),"MNMPRO03",8);
 mnm_ack_put(bytes.data(),2,3);mnm_ack_put(bytes.data(),3,64);mnm_ack_put(bytes.data(),4,0x40209ca7);
 mnm_ack_put(bytes.data(),5,131072);mnm_ack_put(bytes.data(),6,32);
 unsigned sequence=0;
 for(const auto& c:operations){auto fields=c.fields;fields[0]=96+c.bytes().size();fields[1]=++sequence;fields[18]=c.bytes().size();
  const auto at=bytes.size();bytes.resize(at+fields[0]);
  for(unsigned i=0;i<24;++i)mnm_ack_put(bytes.data()+at,i,fields[i]);
  std::copy(c.bytes().begin(),c.bytes().end(),bytes.begin()+at+96);
 }
 return bytes;
}
CanvasProducerStream stream(const std::vector<CanvasProducer>& operations) {
 return decodeCanvasProducers(prefix(operations),false);
}
void state(unsigned char* s,unsigned v){__atomic_store_n(reinterpret_cast<unsigned*>(s),v,__ATOMIC_RELEASE);}
void channelControls(const QString& path) {
 rejected([&]{WorldContinuousChannel::create(path+"bad",1,0,false,false,true);});
 rejected([&]{WorldContinuousChannel::create(path+"bad",1,0,true,true,true);});
 WorldContinuousChannel::create(path,0x200000003ull,0,true,false,true);
 WorldContinuousChannel native(path);require(native.prefixes()&&!native.snapshot(),"V6 negotiation");
 QFile file(path);require(file.open(QIODevice::ReadWrite),"Fixture mapping");auto* m=file.map(0,MNM_CONT_BYTES);require(m,"Fixture map");
 mnm_ack_put(m,9,1);auto* slot=m+MNM_CONT_HEADER;auto* source=slot+MNM_CONT_SLOT_HEADER;
 auto bytes=prefix({operation(3)});
 auto publish=[&](unsigned ordinal){
  std::memset(slot,0,128);mnm_ack_put(slot,2,1);mnm_ack_put(slot,4,bytes.size());
  std::memcpy(source,bytes.data(),bytes.size());mnm_ack_put(slot,6,mnm_cont_crc32c(source,bytes.size()));
  mnm_ack_put(slot,22,ordinal);state(slot,MNM_CONT_SOURCE_REQUEST);
 };
 WorldContinuousChannel::Block b;
 // Check identity/reserved/version/extent guards before acquiring the lease.
 for(unsigned field:{1u,3u,5u,7u,8u,9u,10u,11u,12u,21u,23u,24u,31u}){
  publish(1);mnm_ack_put(slot,field,1);rejected([&]{native.pollSource(b);});
 }
 publish(2);rejected([&]{native.pollSource(b);});
 publish(1);mnm_ack_put(slot,4,64);rejected([&]{native.pollSource(b);});
 publish(1);mnm_ack_put(m,6,9);rejected([&]{native.pollSource(b);});mnm_ack_put(m,6,3);
 publish(1);source[80]^=1;rejected([&]{native.pollSource(b);});
 for(unsigned ordinal=1;ordinal<=32;++ordinal){
  publish(ordinal);require(!native.poll(b),"Source chunk became a World");
  require(native.pollSource(b)&&b.generation==1&&b.request.empty(),"Source chunk acquisition");
  rejected([&]{native.release(1);});
  rejected([&]{native.reply(1,std::vector<std::uint8_t>(80));});
  source[80]^=1;rejected([&]{native.acknowledgeSource(b);});source[80]^=1;
  auto wrong=b;wrong.generation=2;rejected([&]{native.acknowledgeSource(wrong);});
  native.acknowledgeSource(b);require(mnm_ack_word(slot,0)==MNM_CONT_SOURCE_ACK,"Missing chunk ack");
  rejected([&]{native.acknowledgeSource(b);});
 }
 // Same World generation still owns the original slot after32 chunks.
 std::memset(slot,0,128);mnm_ack_put(slot,2,1);mnm_ack_put(slot,4,64);mnm_ack_put(slot,5,64);
 std::memset(source,0,64);auto* request=source+MNM_CONT_SOURCE_BYTES;std::memset(request,0,64);
 mnm_ack_put(slot,6,mnm_cont_crc32c(source,64));mnm_ack_put(slot,7,mnm_cont_crc32c(request,64));state(slot,MNM_CONT_REQUEST);
 require(native.poll(b)&&b.generation==1,"Chunks advanced World ordinal");
 std::vector<std::uint8_t> ack(80);mnm_ack_encode(ack.data(),request,1,0,3,2);native.reply(1,ack);
 mnm_ack_put(slot,8,256);state(slot,MNM_CONT_RETURNED);native.release(1);
 native.cancel();rejected([&]{native.pollSource(b);});require(file.unmap(m),"Fixture unmap");
 // Previous envelopes reject source-only state instead of ignoring it.
 WorldContinuousChannel::create(path+"old",4,0,true);WorldContinuousChannel old(path+"old");
 QFile oldFile(path+"old");require(oldFile.open(QIODevice::ReadWrite),"Old fixture");auto* om=oldFile.map(0,MNM_CONT_BYTES);require(om,"Old map");
 state(om+MNM_CONT_HEADER,MNM_CONT_SOURCE_REQUEST);rejected([&]{old.poll(b);});rejected([&]{old.pollSource(b);});oldFile.unmap(om);
}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir tmp;require(tmp.isValid(),"Fixture root");channelControls(tmp.filePath("channel"));
 const auto root=std::filesystem::path(tmp.path().toStdString());populate(root);
 render::GlBlitter renderer;unsigned total=0;
 {
  WorldRollingSession native(renderer,store(root),false,true);
  CanvasProducerReplay cpu([](const auto&){return std::vector<std::uint8_t>{};});
  for(unsigned chunk=0;chunk<17;++chunk){
   std::vector<CanvasProducer> ops;
   if(!chunk){ops={operation(1),operation(1,9)};auto f=operation(5,9);f.fields[14]=0x4321;ops.push_back(f);}
   else {ops.push_back(operation(2));ops.push_back(operation(1));}
   auto fill=operation(5);fill.fields[14]=0x1234+chunk;ops.push_back(fill);
   auto copy=operation(6,9);copy.fields[4]=7;copy.fields[14]=0xffffffff;
   while(ops.size()<16384)ops.push_back(copy);
   auto decoded=stream(ops);for(const auto& c:decoded.operations)cpu.apply(c);
   native.consumeSource(decoded);total+=decoded.operations.size();
   require(native.completed()==0&&native.worldReadbacks()==0,"Source-only World credit/readback");
  }
  // Build a real small World using a known closed fixture sprite.
  const auto spr=sprite();const unsigned at=36,size=55;
  auto raster=operation(9,9);raster.fields[8]=2;raster.fields[14]=2;raster.fields[15]=8;raster.fields[19]=size;raster.fields[18]=size;
  raster.payload.assign(spr.begin()+at,spr.begin()+at+size);std::fill(raster.payload.begin()+28,raster.payload.begin()+32,0);
  auto entry=operation(11,9);entry.fields[14]=1;auto check=entry;check.fields[2]=10;check.fields[15]=4;auto returned=entry;returned.fields[2]=12;
  const auto world=decodeRollingQueue(encodeRollingQueue({entry,raster,check,returned}));
  for(const auto& c:world.operations)cpu.apply(c);
  auto frame=native.consumeResident(1,world);require(frame.valid()&&native.completed()==1,"World after chunks");
  require(native.worldReadbacks()==0&&native.diagnosticReadResident(renderer).pixels==cpu.read(9).pixels,"Source history lost across chunks");
 }
 require(renderer.stats().surfaces==0,"Source chunk teardown leaked resources");
 {
  WorldRollingSession invalid(renderer,store(root),false,true);
  auto badEntry=operation(11);badEntry.fields[14]=1;
  rejected([&]{invalid.consumeSource(stream({operation(1),badEntry}));});
  require(invalid.failed()&&renderer.stats().surfaces==0,"Malformed chunk partially applied");
  rejected([&]{invalid.consumeSource(stream({operation(1)}));});
 }
 std::cout<<"{\"source_chunks\":17,\"source_records\":"<<total<<",\"channel_chunks\":32,\"worlds\":1,\"remaining_surfaces\":0}"<<std::endl;
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}
