#include "live_command_renderer.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_stream_v5.h"
#include "../protocols/include/mnm/native_cpu_read_v1.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void word(QByteArray& b,unsigned v){char bytes[4];qToLittleEndian(v,bytes);b.append(bytes,4);}
static QByteArray fields(std::initializer_list<unsigned> values){QByteArray b;for(auto v:values)word(b,v);return b;}
static void record(QByteArray& b,unsigned seq,unsigned op,const QByteArray& data){word(b,op);word(b,seq);word(b,unsigned(data.size()));b+=data;}
static QByteArray create(unsigned id){auto b=fields({id,800,600,16,0xf800,0x7e0,31});b+=QByteArray(800*600*2,0);return b;}
int main(int argc,char** argv)try{
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
 QApplication app(argc,argv);GlViewport viewport;viewport.resize(320,240);viewport.show();QElapsedTimer wait;wait.start();
 while(!viewport.ready()&&wait.elapsed()<10000)app.processEvents();
 check(viewport.ready(),"GL context");
 QTemporaryDir out;check(out.isValid(),"reply directory");
 GlBlitter world(viewport.context());Image expected{800,600,std::vector<std::uint32_t>(800*600)};
 for(unsigned i=0;i<expected.pixels.size();++i)expected.pixels[i]=(i*71+19)&65535;
 const auto id=world.create(expected,{16,{0xf800,0x7e0,31}});auto frame=world.presentGpu(id);
 std::vector<std::uint64_t> pending;
 LiveCommandRenderer renderer(viewport);renderer.cpuReadPending=[&](auto ticket){pending.push_back(ticket);};renderer.residentWorld=[&](auto generation,auto nonce){check(generation==1&&nonce==0x600000005ULL,"World identity");return frame;};
 check(renderer.create(out.filePath("commands"),501,2),"command channel");
 QFile producer(out.filePath("commands"));check(producer.open(QIODevice::ReadWrite),"producer file");auto* map=producer.map(0,MNM_RENDER_COMMANDS_V2_SIZE);
 mnm_ring_writer writer{};check(mnm_ring_writer_bind(&writer,map,MNM_RENDER_COMMANDS_V2_SIZE),"producer claim");
 const auto publish=[&](const QByteArray& bytes){
  for(qsizetype at=0;at<bytes.size();){const auto count=unsigned(qMin<qsizetype>(32768,bytes.size()-at));const int status=mnm_ring_write(&writer,bytes.constData()+at,count);check(status>=0,"producer rejected");if(status)at+=count;check(renderer.poll(),"native command failed");app.processEvents();}
  for(unsigned i=0;i<128&&renderer.hasPendingCommands();++i)check(renderer.poll(),"native command tail failed");
 };
 QByteArray first("MNMCMD05",8);word(first,5);word(first,16);
 record(first,1,1,create(1));record(first,2,1,create(2));record(first,3,19,fields({1,1,0,5,6}));
 auto patch=fields({1,19,37,2,1});patch+=QByteArray::fromHex("00f8cdab0100");record(first,4,20,patch);expected.pixels[37*800+19]=0xf800;
 record(first,5,3,fields({1,2,0,0,800,600,0,0,0,0}));record(first,6,6,fields({2}));
 const std::uint32_t request[]={2,1,0,501,800,600,1,0,5,6};
 record(first,7,21,fields({2,1,0,501,800,600,1,0,5,6}));publish(first);
 const auto verify=[&](unsigned ticket){
  QFile file(out.filePath("commands.cpu-reply"));check(file.open(QIODevice::ReadOnly),"atomic reply missing");const auto b=file.readAll();check(b.size()==64+800*600*2,"reply length");
  std::uint32_t h[16],r[10];for(unsigned i=0;i<16;++i)h[i]=qFromLittleEndian<quint32>(b.constData()+i*4);for(unsigned i=0;i<10;++i)r[i]=request[i];r[1]=ticket;
  check(mnm_native_cpu_read_reply(h,unsigned(b.size()),r),"reply binding");
  for(unsigned i=0;i<expected.pixels.size();++i)check(qFromLittleEndian<quint16>(b.constData()+64+i*2)==expected.pixels[i],"native CPU words differ");
  for(unsigned i=0;i<16;++i){auto saved=h[i];h[i]^=1;check(!mnm_native_cpu_read_reply(h,unsigned(b.size()),r),"malformed reply accepted");h[i]=saved;}
  check(!mnm_native_cpu_read_reply(h,unsigned(b.size())-1,r)&&!mnm_native_cpu_read_reply(h,unsigned(b.size())+1,r),"truncated/extra reply accepted");
  check(renderer.result()->cpuMaterializations==ticket&&renderer.result()->stats.nativeReadbacks==ticket&&world.stats().nativeReadbacks==0,"explicit/ordinary readback accounting");
 };
 verify(1);check(pending==std::vector<std::uint64_t>{1,0},"CPU wait progress pair");QByteArray next;auto change=fields({2,0,0,1,1});change+=QByteArray::fromHex("e007");record(next,8,2,change);expected.pixels[0]=0x7e0;
 record(next,9,21,fields({2,2,0,501,800,600,1,0,5,6}));publish(next);verify(2);check(pending==std::vector<std::uint64_t>{1,0,2,0},"Second CPU wait pair");
 unsigned refused=0;
 for(unsigned mode=0;mode<5;++mode){
  try{CommandDecoder decoder;decoder.append(first);auto bad=fields({mode==1?999u:2u,mode==0?1u:2u,0,501,mode==2?799u:800u,600,1,0,mode==3?0u:5u,mode==3?0u:6u});QByteArray packet;record(packet,8,21,bad);if(mode==4){auto old=first;old[7]='4';old[8]=4;CommandDecoder legacy;legacy.append(old);}else decoder.append(packet);throw std::runtime_error("bad CPU request admitted");}
  catch(const std::runtime_error& e){if(QByteArray(e.what())=="bad CPU request admitted")throw;++refused;}
 }
 QByteArray wrong;record(wrong,10,21,fields({2,3,0,502,800,600,1,0,5,6}));check(mnm_ring_write(&writer,wrong.data(),unsigned(wrong.size()))==1,"wrong-session publish");check(!renderer.poll()&&renderer.result()->liveSurfaces==0&&renderer.result()->stats.nativeReadbacks==2,"wrong session read or leaked");
 renderer.abort();producer.unmap(map);frame={};world.destroy(id);
 std::printf("{\"success\":true,\"native_words\":960000,\"explicit_readbacks\":2,\"ordinary_world_readbacks\":0,\"wire_rejections\":%u,\"malformed_replies\":36,\"wrong_session_before_readback\":true,\"remaining_surfaces\":0}\n",refused);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
