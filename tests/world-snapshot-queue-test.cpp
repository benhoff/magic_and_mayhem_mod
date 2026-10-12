#include "../compat/legacy/world_snapshot_queue.hpp"
#include "../protocols/include/mnm/world_continuous_checksum.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <thread>
using namespace mnm::legacy;
namespace {
using Clock=std::chrono::steady_clock;
void put(unsigned char* p,unsigned n,unsigned v){for(unsigned i=0;i<4;++i)p[4*n+i]=static_cast<unsigned char>(v>>(i*8));}
unsigned get(const unsigned char* p,unsigned n){return mnm_ack_word(p,n);}
unsigned load(const unsigned char* p){return __atomic_load_n(reinterpret_cast<const unsigned*>(p),__ATOMIC_ACQUIRE);}
void store(unsigned char* p,unsigned n){__atomic_store_n(reinterpret_cast<unsigned*>(p),n,__ATOMIC_RELEASE);}
template<class F> void wait(F f){auto start=Clock::now();while(!f()){assert(Clock::now()-start<std::chrono::seconds(10));std::this_thread::sleep_for(std::chrono::milliseconds(1));}}
template<class F> void refuses(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}assert(caught);}
std::vector<unsigned char> read(const char* path){QFile f(QString::fromLocal8Bit(path));assert(f.open(QIODevice::ReadOnly));auto b=f.readAll();return {b.begin(),b.end()};}
struct Peer {
 QFile file;unsigned char* map;std::vector<unsigned char> source,request;unsigned closes=0,oracle=0;
 Peer(const QString& path,const std::vector<unsigned char>& s,const std::vector<unsigned char>& r):file(path),source(s),request(r){
  assert(file.open(QIODevice::ReadWrite));map=file.map(0,MNM_CONT_BYTES);assert(map);store(map+36,1);
  for(std::size_t at=64;at<source.size();){assert(source.size()-at>=96);if(get(source.data()+at,2)==10)oracle=std::max(oracle,get(source.data()+at,14));at+=96+get(source.data()+at,18);++closes;assert(at<=source.size());}
 }
 ~Peer(){assert(file.unmap(map));}
 unsigned char* slot(unsigned g){return map+MNM_CONT_HEADER+((g-1)&1)*MNM_CONT_SLOT_BYTES;}
 void publish(unsigned g,bool badSource=false,bool badRequest=false){
  auto* p=slot(g);assert(load(p)==0);std::memset(p+4,0,124);put(p,2,g);put(p,4,unsigned(source.size()));put(p,5,unsigned(request.size()));
  std::memcpy(p+128,source.data(),source.size());auto* r=p+128+MNM_CONT_SOURCE_BYTES;std::memcpy(r,request.data(),request.size());
  if(badRequest)r[48]^=1;
  put(p,6,mnm_cont_crc32c(p+128,unsigned(source.size()))^(badSource?1:0));put(p,7,mnm_cont_crc32c(r,unsigned(request.size())));store(p,2);
 }
 void returned(unsigned g,int mutation=-1,bool prefix=false){
  auto* p=slot(g);wait([&]{return load(p)==4;});
  auto* ack=p+128+MNM_CONT_SOURCE_BYTES+MNM_CONT_REQUEST_BYTES;
  assert(!std::memcmp(ack,"MNMSACK1",8)&&get(ack,18)==2&&get(ack,14)==g);
  assert(get(p,10)==80&&get(p,11)==mnm_cont_crc32c(ack,80));
  unsigned c[48]={};c[0]=c[24]=96;c[1]=closes+1;c[2]=10;c[3]=get(request.data(),7);c[5]=get(request.data(),8);c[6]=get(request.data(),9);c[7]=get(request.data(),10);c[14]=oracle+1;c[15]=4;c[16]=get(request.data(),5);
  for(unsigned i=0;i<24;++i){c[24+i]=c[i];}
  c[25]++;c[26]=12;c[38]=get(request.data(),5);c[39]=get(request.data(),15);c[40]=0;c[41]=get(request.data(),4);c[43]=get(request.data(),14);c[44]=2;
  auto* tail=p+128+source.size();for(unsigned i=0;i<48;++i)put(tail,i,c[i]);if(mutation>=0)tail[mutation]^=1;if(prefix)p[128+source.size()-1]^=1;
  put(p,8,unsigned(source.size()+192));put(p,12,g*50);put(p,13,g*50+2);put(p,14,g*50+3);put(p,15,g*50+4);store(p,5);
 }
};
}
int main(int argc,char** argv){
 QCoreApplication app(argc,argv);assert(argc==3);auto source=read(argv[1]),request=read(argv[2]);QTemporaryDir temp;assert(temp.isValid());
 {
  auto path=temp.filePath("owned");WorldContinuousChannel::create(path,0x100000007ULL,0,true,true);WorldContinuousChannel channel(path);assert(channel.snapshot()&&channel.resident());Peer peer(path,source,request);WorldSnapshotQueue q(channel);
  peer.publish(1);peer.returned(1);wait([&]{return q.stats().returned==1;});peer.publish(2);peer.returned(2);wait([&]{return q.stats().returned==2;});
  // Rendering is deliberately absent: both accepted sources already belong to
  // native storage and original returns have been verified.
  assert(q.stats().outstanding==2&&q.stats().highWater==2);peer.publish(3);std::this_thread::sleep_for(std::chrono::milliseconds(30));assert(load(peer.slot(3))==2&&q.stats().accepted==2);
  OwnedWorldSnapshot a,b,c;assert(q.take(a)&&q.take(b)&&!q.take(c));assert(a.generation==1&&b.generation==2);
  auto frozen=encodeRollingQueue(a.handoff.stream.operations);std::memset(peer.slot(2)+128,0x75,source.size());assert(encodeRollingQueue(a.handoff.stream.operations)==frozen);
  refuses([&]{q.imported(2);});q.imported(1);peer.returned(3);wait([&]{return q.stats().returned==3;});assert(q.take(c)&&c.generation==3);refuses([&]{q.imported(1);});q.imported(2);q.imported(3);assert(q.stats().outstanding==0&&q.stats().imports==3&&q.stats().bytes==3*source.size());q.stop();assert(q.stats().terminal);
 }
 for(int failure=0;failure<12;++failure){
  auto path=temp.filePath(QString("fail-%1").arg(failure));WorldContinuousChannel::create(path,7,0,true,true);WorldContinuousChannel channel(path);Peer peer(path,source,request);WorldSnapshotQueue q(channel);peer.publish(1,failure==0,failure==1);
  if(failure>=2){const int changes[]={0,4,8,12,56,60,64,96+76,96+80};peer.returned(1,failure==11?-1:changes[failure-2],failure==11);}
  wait([&]{return q.stats().terminal;});assert(!q.stats().error.empty()&&channel.cancelled()&&q.stats().returned==0);OwnedWorldSnapshot bad;refuses([&]{q.take(bad);});
 }
 {
  auto path=temp.filePath("cancel");WorldContinuousChannel::create(path,9,0,true,true);WorldContinuousChannel channel(path);WorldSnapshotQueue q(channel);channel.cancel();wait([&]{return q.stats().terminal;});assert(q.stats().accepted==0);
 }
 {
  auto path=temp.filePath("legacy");WorldContinuousChannel::create(path,11,0,true);WorldContinuousChannel channel(path);refuses([&]{WorldSnapshotQueue q(channel);});
 }
 std::cout<<"{\"owned_before_render\":2,\"backpressure\":true,\"source_overwrite_safe\":true,\"ordered_imports\":3,\"failure_cases\":12,\"legacy_refused\":true,\"cancellation_joined\":true}\n";
}
