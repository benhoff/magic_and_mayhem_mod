#include "surface_backend.hpp"
#include "../reconstruction/rendering/surface_bitmap.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace mnm::render;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned guards=0;
template<class F>static void refuses(F&& f){bool caught=false;try{f();}catch(const std::runtime_error&){caught=true;}require(caught,"Expected owned refusal");++guards;}
static Rect rect(const QJsonArray& a){require(a.size()==4,"Rect extent");return {a[0].toInt(),a[1].toInt(),a[2].toInt(),a[3].toInt()};}
static Image image(const QJsonArray& a){require(a.size()==48,"Image extent");Image i{8,6,{}};for(auto v:a)i.pixels.push_back(std::uint32_t(v.toDouble()));return i;}
static QJsonArray words(const Image& i){QJsonArray a;for(auto v:i.pixels)a.append(qint64(v));return a;}
static QJsonArray desc(SurfaceBackend::Descriptor d){d[9]=d[9]==0xabababab?0:d[9]?2:1;QJsonArray a;for(auto v:d)a.append(qint64(v));return a;}
static PixelFormat format(unsigned bits){return {bits,bits==8?std::array<std::uint32_t,3>{}:bits==16?std::array<std::uint32_t,3>{0xf800,0x7e0,31}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff}};}
static ClipperState clip(unsigned n){ClipperState c;if(n){c.attached=true;if(n!=3){c.regions=std::vector<Rect>{};if(n==1)*c.regions={{2,1,6,5}};else if(n==2)*c.regions={{0,0,3,2},{5,3,8,6}};else if(n==5)*c.regions={{0,0,8,3},{0,4,8,6}};else require(n==4,"Clip kind");}}return c;}
static SurfaceBackend::Palette palette(unsigned style){SurfaceBackend::Palette p{};for(unsigned i=0;i<256;++i){unsigned r=i,g=i,b=i;if(style==2)r=g=b=255-i;if(style==3){r=((i>>5)&7)*255/7;g=((i>>2)&7)*255/7;b=(i&3)*85;if(i==255)r=g=b=0;}p[i]={std::uint8_t(r),std::uint8_t(g),std::uint8_t(b)};}return p;}
static QJsonArray table(const SurfaceBackend& b,SurfaceId id,unsigned bits){QJsonArray out;SurfaceBackend::Palette p{};if(bits==8)p=b.dcPalette(id);for(auto c:p)out.append(qint64((unsigned(c.red)<<16)|(unsigned(c.green)<<8)|c.blue));return out;}
static void policies(SurfaceBackend& b,const SurfaceBackend::Palette& defaults){
 auto s=b.create({8,6,std::vector<std::uint32_t>(48,123)},format(16));SurfaceBackend::Descriptor d;d.fill(0xabababab);d[0]=108;
 require(b.lock(s,d)==0,"CPU lease");refuses([&]{b.destroy(s);});refuses([&]{b.read(s);});refuses([&]{b.update(s,0,0,{1,1,{9}});});
 b.writeLocked(s,2,1,{1,1,{456}});require(b.lockedPixels(s).pixels[10]==456,"CPU lease mirror");refuses([&]{b.writeLocked(s,-1,0,{1,1,{9}});});
 std::uint32_t dc=0;require(b.acquireDc(s,dc)==0,"DC during CPU lease");refuses([&]{b.writeLocked(s,0,0,{1,1,{9}});});refuses([&]{b.readDc(s);});
 require(b.releaseDc(s,dc)==0&&b.unlock(s)==0&&b.read(s).pixels[10]==456,"CPU commit");
 b.invalidateContents(s);d.fill(0xabababab);d[0]=108;refuses([&]{b.lock(s,d);});require(!b.borrowed(s)&&d[1]==0xabababab,"Unknown lock mutation");refuses([&]{b.presentGpu(s);});
 b.fill(s,Rect{0,0,4,6},789);refuses([&]{b.read(s);});b.fill(s,Rect{4,0,8,6},789);require(b.read(s).pixels==std::vector<std::uint32_t>(48,789),"Fill did not define unknown bytes");
 b.setClipper(s,clip(1));refuses([&]{b.setClipper(s,{true,std::vector<Rect>{{-1,0,2,2}}});});require(b.fill(s,std::nullopt,321).pieces==1,"Rejected clip mutated state");b.destroy(s);refuses([&]{b.read(s);});
 auto i=b.create({8,6,std::vector<std::uint32_t>(48,19)},format(8));dc=0xabababab;refuses([&]{b.acquireDc(i,dc);});require(dc==0xabababab&&!b.borrowed(i),"Missing default admission mutation");b.bindPalette(i,palette(1));require(b.acquireDc(i,dc)==0,"Explicit palette admission");refuses([&]{b.bindPalette(i,std::nullopt);});refuses([&]{b.setDcColors(i,255,{{0,0,0},{1,1,1}});});require(b.dcPalette(i)[255].red==255,"Rejected table edit");require(b.releaseDc(i,dc)==0,"Release palette DC");b.destroy(i);
 auto p=b.create({8,6,std::vector<std::uint32_t>(48,19)},format(8),0x840,defaults);b.bindPalette(p,palette(1));require(b.acquireDc(p,dc)==0,"DC snapshot");b.bindPalette(p,palette(3));require(b.dcPalette(p)[64].red==64,"Binding changed active snapshot");require(b.releaseDc(p,0)==SurfaceAccessState::badDc&&b.borrowed(p),"Bad token released ownership");require(b.releaseDc(p,dc)==0,"Palette DC release");const auto displayed=b.present(p).pixel(0,0)&0xffffff;auto c=palette(3)[19];require(displayed==((unsigned(c.red)<<16)|(unsigned(c.green)<<8)|c.blue),"Deferred display binding");b.destroy(p);
 refuses([&]{b.create({1,1,{0}},{16,{0x7c00,0x3e0,31}});});
 auto large=b.create({70,70,std::vector<std::uint32_t>(4900,123)},format(16));b.setSourceKey(large,321);SurfaceCopyRequest r;r.source=r.destination={0,0,70,70};r.flags=0x8000;refuses([&]{b.copy(large,large,r);});require(b.read(large).pixels==std::vector<std::uint32_t>(4900,123),"Overlap budget mutated bytes");b.destroy(large);
 require(b.stats().surfaces==0,"Policy resource leak");
}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{
 require(argc==4,"Expected input/assets/output");QFile in(QString::fromLocal8Bit(argv[1]));require(in.open(QIODevice::ReadOnly),"Input file");QJsonParseError error;auto input=QJsonDocument::fromJson(in.readAll(),&error).object();require(error.error==QJsonParseError::NoError,"Input JSON");SurfaceBackend::Palette defaults{};auto values=input["defaults"].toArray();require(values.size()==256,"Default context");for(unsigned i=0;i<256;++i){auto v=unsigned(values[int(i)].toDouble());defaults[i]={std::uint8_t(v>>16),std::uint8_t(v>>8),std::uint8_t(v)};}
 auto backend=std::make_unique<SurfaceBackend>();auto& b=*backend;policies(b,defaults);QJsonArray draws;SurfaceId s=0,d=0;
 for(auto v:input["draws"].toArray()){auto c=v.toObject();const bool fill=c["kind"]=="fill";const bool hasSource=!fill||c["aux_source"].toBool();
  if(!c["continues"].toBool()){if(hasSource)s=b.create(image(c["source_pixels"].toArray()),format(16));d=c["shared"].toBool()?s:b.create(image(c["destination_pixels"].toArray()),format(16));}
  else require(b.read(s).pixels==image(c["source_pixels"].toArray()).pixels&&b.read(d).pixels==image(c["destination_pixels"].toArray()).pixels,"Recorded continuation inputs differ");
  b.setClipper(d,clip(c["clip"].toInt()));if(hasSource)b.setSourceKey(s,c["key"].isNull()?std::nullopt:std::optional<std::uint16_t>(c["key"].toInt()));
  SurfaceBackend::Descriptor descriptor;descriptor.fill(0xabababab);descriptor[0]=108;auto held=c["held"].toInt();std::uint32_t heldDc=0;
  if(held==1||held==2||held==5)require(b.lock(held==1?s:d,descriptor)==0,"Owned held-lock setup");
  if(held==3||held==4||held==5)require(b.acquireDc(held==3?s:d,heldDc)==0,"Owned held-DC setup");
  const auto counters=b.stats();
  SurfaceCopyResult result;if(fill)result=b.fill(d,c["null_rect"].toBool()?std::nullopt:std::optional(rect(c["destination"].toArray())),std::uint16_t(c["color"].toDouble()),unsigned(c["flags"].toDouble()));
  else{SurfaceCopyRequest r;r.api=c["fast"].toBool()?SurfaceCopyApi::BltFast:SurfaceCopyApi::Blt;r.source=rect(c["source"].toArray());r.destination=rect(c["destination"].toArray());r.flags=unsigned(c["flags"].toDouble());result=b.copy(s,d,r);}
  const auto after=b.stats();if(!fill)require(after.uploads==counters.uploads&&after.nativeReadbacks==counters.nativeReadbacks,"Resident copy uploaded/read pixels");
  QJsonArray lockedAfter;if(fill&&(held==2||held==5))lockedAfter=words(b.lockedPixels(d));
  if(held==3||held==4||held==5)require(b.releaseDc(held==3?s:d,heldDc)==0,"Owned held-DC cleanup");
  if(held==1||held==2||held==5)require(b.unlock(held==1?s:d)==0,"Owned held-lock cleanup");
  QJsonObject out{{"tag",c["tag"]},{"hresult",qint64(result.hresult)},{"destination_after",words(b.read(d))}};if(hasSource)out["source_after"]=words(b.read(s));if(!lockedAfter.isEmpty())out["locked_after"]=lockedAfter;draws.append(out);
  if(!c["keep"].toBool()){b.destroy(d);if(hasSource&&s!=d)b.destroy(s);s=d=0;}
 }
 require(b.stats().surfaces==0,"Draw resources");QJsonArray reuse;
 for(auto v:input["reuse"].toArray()){auto c=v.toObject();unsigned bits=c["bits"].toInt(),action=c["action"].toInt();bool initial=c["initial_palette"].toBool();const auto id=b.create({8,6,std::vector<std::uint32_t>(48,bits==8?19:bits==16?0x2bab:0x556677)},format(bits),0x840,bits==8?std::optional(defaults):std::nullopt);if(initial)b.bindPalette(id,palette(1));
  std::ifstream asset(std::string(argv[2])+"/"+c["asset"].toString().toStdString()+".bmp",std::ios::binary);std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(asset),{}};auto parsed=mnm::reconstruction::parseSurfaceBitmap(bytes);require(bool(parsed),"Bitmap input");QJsonArray phases;
  for(unsigned phase=0;phase<3;++phase){if(phase==1){if(action==1&&initial){auto p=palette(2);b.updatePalette(id,64,{p.begin()+64,p.begin()+96});}if(action==2)b.bindPalette(id,palette(3));if(action==3)b.bindPalette(id,std::nullopt);}
   std::uint32_t dc=0xabababab;require(b.acquireDc(id,dc)==0&&!b.dcRegions(id),"Fresh owned lease/clip");QJsonObject out{{"table_before",table(b,id,bits)}};
   if(phase==0){if(action==4){std::vector<Rgb> p;for(unsigned i=64;i<96;++i)p.push_back({std::uint8_t(255-i),std::uint8_t(i*5),std::uint8_t(i*7)});b.setDcColors(id,64,p);}if(action==5)b.bindPalette(id,palette(3));if(action==6&&initial){auto p=palette(2);b.updatePalette(id,64,{p.begin()+64,p.begin()+96});}if(c["clip"].toInt())b.selectDcRegions(id,std::vector<Rect>{{2,1,6,5}});}
   if(phase==2){b.selectDcRegions(id,std::nullopt);}
   out["table_draw"]=table(b,id,bits);b.reloadDib(id,*parsed);out["pixels"]=words(b.readDc(id));auto rgb=b.presentDc(id);QJsonArray colors;for(int y=0;y<6;++y)for(int x=0;x<8;++x)colors.append(qint64(rgb.pixel(x,y)&0xffffff));out["rgb"]=colors;require(b.releaseDc(id,dc)==0,"Owned releaseDC");phases.append(out);
  }
  reuse.append(phases);b.destroy(id);
 }
 QJsonArray access;unsigned ownerResets=0;
 for(auto v:input["access"].toArray()){auto c=v.toObject();auto& owner=*backend;unsigned bits=c["bits"].toInt();Image pixels{8,6,{}};for(unsigned i=0;i<48;++i)pixels.pixels.push_back((i*17+3)&(bits==8?255:bits==16?65535:0xffffffffu));auto id=owner.create(pixels,format(bits),c["caps"].toInt(),bits==8?std::optional(defaults):std::nullopt);std::uint32_t saved=0;QJsonArray steps;
  for(auto operation:c["operations"].toArray()){unsigned op=operation.toInt();SurfaceBackend::Descriptor descriptor;descriptor.fill(0xffffffff);unsigned result=0,out=0xffffffff;
   if(op==0){descriptor.fill(0xabababab);descriptor[0]=108;result=owner.lock(id,descriptor);}else if(op==1)result=owner.unlock(id);else if(op==2){unsigned dc=0xabababab;result=owner.acquireDc(id,dc);out=dc==0xabababab?0:dc?2:1;if(!result)saved=dc;}else result=owner.releaseDc(id,op==3?saved:op==4?0:0xfefefefe);
   steps.append(QJsonObject{{"result",qint64(result)},{"dc_out_state",qint64(out)},{"descriptor",op==0?desc(descriptor):QJsonArray{}}});
  }
  SurfaceBackend::Descriptor descriptor;descriptor.fill(0xabababab);descriptor[0]=108;auto result=owner.lock(id,descriptor);QJsonObject final{{"result",qint64(result)},{"descriptor",desc(descriptor)}};QJsonValue wordsAfter;
  if(!result){wordsAfter=words(owner.lockedPixels(id));require(owner.unlock(id)==0,"Terminal unlock");owner.destroy(id);}else{require(owner.poisoned(id),"Unexplained terminal busy");refuses([&]{owner.destroy(id);});refuses([&]{owner.read(id);});backend=std::make_unique<SurfaceBackend>();++ownerResets;}
  access.append(QJsonObject{{"steps",steps},{"final_lock",final},{"pixels",wordsAfter}});
 }
 require(backend->stats().surfaces==0,"Final resources");auto driver=backend->driver();QJsonObject output{{"draws",draws},{"reuse",reuse},{"access",access},{"guards",int(guards)},{"owner_resets",int(ownerResets)},{"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)}};QFile out(QString::fromLocal8Bit(argv[3]));require(out.open(QIODevice::WriteOnly|QIODevice::NewOnly),"New output");auto bytes=QJsonDocument(output).toJson(QJsonDocument::Compact);require(out.write(bytes)==bytes.size(),"Output write");std::cout<<"{\"success\":true,\"draws\":"<<draws.size()<<",\"reuse_cases\":"<<reuse.size()<<",\"access_cases\":"<<access.size()<<",\"guards\":"<<guards<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
