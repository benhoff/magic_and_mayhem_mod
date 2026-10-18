#include "live_command_renderer.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/native_cpu_read_v1.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
using namespace mnm::render;
static void check(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
static unsigned word(const QByteArray& b,qsizetype at){check(at>=0&&at+4<=b.size(),"word bounds");return qFromLittleEndian<quint32>(b.constData()+at);}
int main(int argc,char** argv)try{
 check(argc==2,"Expected retained PE32 command archive");
 QFile input(QString::fromLocal8Bit(argv[1]));check(input.open(QIODevice::ReadOnly),"archive open");auto bytes=input.readAll();
 check(bytes.size()>=16&&bytes.left(8)=="MNMCMD05"&&word(bytes,8)==5&&word(bytes,12)==16,"archive header");
 QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
 QApplication app(argc,argv);GlViewport viewport;viewport.show();QElapsedTimer wait;wait.start();while(!viewport.ready()&&wait.elapsed()<10000)app.processEvents();check(viewport.ready(),"GL context");
 GlBlitter world(viewport.context()),inspector(viewport.context());std::vector<SurfaceId> handles;std::vector<GpuFrame> frames;
 for(unsigned generation=1;generation<=19;++generation){Image image{800,600,std::vector<std::uint32_t>(800*600)};for(unsigned i=0;i<image.pixels.size();++i)image.pixels[i]=(i*71+generation*19)&65535;
  auto id=world.create(image,{16,{0xf800,0x7e0,31}});handles.push_back(id);frames.push_back(world.presentGpu(id));}
 Image expected{800,600,std::vector<std::uint32_t>(800*600)};auto observed=inspector.create(expected,{16,{0xf800,0x7e0,31}});GpuFrame presented;
 NativeRenderTarget target{[&]{return viewport.ready();},[&]{return viewport.context();},[&](GpuFrame frame){presented=std::move(frame);},[&]{return viewport.error();},nullptr,nullptr};
 LiveCommandRenderer renderer(target);renderer.residentWorld=[&](std::uint64_t g,std::uint64_t n){check(g>=1&&g<=19&&n==0x2200000011ULL,"World binding");return frames[g-1];};
 QTemporaryDir out;check(out.isValid()&&renderer.create(out.filePath("commands"),123,2),"native channel");QFile producer(out.filePath("commands"));check(producer.open(QIODevice::ReadWrite),"producer open");auto* map=producer.map(0,MNM_RENDER_COMMANDS_V2_SIZE);mnm_ring_writer writer{};check(mnm_ring_writer_bind(&writer,map,MNM_RENDER_COMMANDS_V2_SIZE),"producer bind");
 const auto publish=[&](const QByteArray& data){for(qsizetype at=0;at<data.size();){unsigned n=unsigned(qMin<qsizetype>(32768,data.size()-at));int wrote=mnm_ring_write(&writer,data.constData()+at,n);check(wrote>=0,"ring write");if(wrote)at+=n;check(renderer.poll(),"native execution");app.processEvents();}for(unsigned i=0;i<256&&renderer.hasPendingCommands();++i)check(renderer.poll(),"native tail");check(!renderer.hasPendingCommands(),"native tail stalled");};
 publish(bytes.left(16));unsigned reads=0,presents=0,worlds=0;qsizetype at=16;
 while(at<bytes.size()){
  check(at+12<=bytes.size(),"record header");unsigned op=word(bytes,at),size=word(bytes,at+8);check(size<=unsigned(bytes.size()-at-12),"record size");auto record=bytes.mid(at,12+size);auto data=record.mid(12);
  if(op==19){unsigned g=word(data,4);for(unsigned i=0;i<expected.pixels.size();++i)expected.pixels[i]=(i*71+g*19)&65535;++worlds;}
  if(op==20){unsigned x=word(data,4),y=word(data,8),w=word(data,12),h=word(data,16),count=w*h;check(size==20+count*3,"mask payload");for(unsigned row=0;row<h;++row)for(unsigned col=0;col<w;++col){unsigned p=row*w+col;if(data[20+count*2+p])expected.pixels[(y+row)*800+x+col]=qFromLittleEndian<quint16>(data.constData()+20+p*2);}}
  publish(record);
  if(op==21){QFile reply(out.filePath("commands.cpu-reply"));check(reply.open(QIODevice::ReadOnly),"native reply");auto content=reply.readAll();check(content.size()==64+800*600*2,"reply size");std::uint32_t h[16],r[10];for(unsigned i=0;i<16;++i)h[i]=word(content,i*4);for(unsigned i=0;i<10;++i)r[i]=word(data,i*4);check(mnm_native_cpu_read_reply(h,unsigned(content.size()),r),"reply identity");for(unsigned i=0;i<expected.pixels.size();++i)check(qFromLittleEndian<quint16>(content.constData()+64+i*2)==expected.pixels[i],"GPU reader words");++reads;}
  if(op==6){check(presented.valid(),"native PRESENT");inspector.importGpu565(observed,presented);check(inspector.read(observed).pixels==expected.pixels,"merged presentation words");++presents;}
  at+=12+size;
 }
 check(mnm_ring_end(&writer)&&renderer.finishProducer(),"producer completion");
 check(worlds==19&&reads==19&&presents==20,"control counts");check(renderer.ended()&&renderer.result()->liveSurfaces==0,"native retirement");check(renderer.result()->cpuMaterializations==19&&renderer.result()->stats.nativeReadbacks==19&&world.stats().nativeReadbacks==0,"reader accounting");
 renderer.abort();producer.unmap(map);presented={};frames.clear();for(auto id:handles)world.destroy(id);inspector.destroy(observed);
 std::printf("{\"success\":true,\"worlds\":%u,\"explicit_reader_snapshots\":%u,\"presentations_verified\":%u,\"reader_words\":9120000,\"presentation_words\":9600000,\"ordinary_world_readbacks\":0,\"diagnostic_presentation_readbacks\":%u,\"remaining_surfaces\":0}\n",worlds,reads,presents,presents);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
