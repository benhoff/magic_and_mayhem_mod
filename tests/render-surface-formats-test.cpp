#include "surface_backend.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <limits>
using namespace mnm::render;
namespace {
unsigned checks=0,refusals=0;
void check(bool ok){++checks;if(!ok)throw std::runtime_error("Format/row check "+std::to_string(checks));}
template<class F> void refuse(F f){bool rejected=false;try{f();}catch(const std::runtime_error&){rejected=true;}check(rejected);++refusals;}
const PixelFormat formats[5]={{8,{}},{16,{0x7c00,0x3e0,31}},{16,{0xf800,0x7e0,31}},{24,{0xff0000,0xff00,255}},{32,{0xff0000,0xff00,255}}};
std::uint32_t key(unsigned f){const std::uint32_t values[]={0x81,0x9234,0x9234,0x901234,0x7f901234};return values[f];}
std::uint32_t mask(unsigned f){return f==0?255:formats[f].masks[0]|formats[f].masks[1]|formats[f].masks[2];}
Image image(unsigned f,bool destination){Image out{7,5,{}};for(unsigned i=0;i<35;++i){auto v=destination?0xa0b0c0d0+i*73:key(f)^(0x1234+i*73);if(!destination){if(i%4==0)v=key(f);else if(i%4==1)v=f==0?0x82:f==1?key(f)^0x8000:f==4?key(f)^0xff000000:key(f)^1;}out.pixels.push_back(v&(formats[f].bits==32?UINT32_MAX:(1u<<formats[f].bits)-1));}return out;}
PixelRows rows(unsigned,int pitch){return {7,5,pitch,std::size_t(16+(pitch<0?4*-pitch:0)),std::vector<std::uint8_t>(256,0xcd)};}
QJsonArray words(const std::vector<std::uint32_t>& v){QJsonArray a;for(auto x:v)a.append(qint64(x));return a;}
QJsonArray descriptor(SurfaceBackend& b,SurfaceId id){SurfaceBackend::Descriptor d{};check(b.lock(id,d)==0);check(b.unlock(id)==0);d[9]=0;return words({d.begin(),d.end()});}
QJsonArray rgba(const QImage& image){QJsonArray a;for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x){const auto* p=image.constScanLine(y)+4*x;a.append(qint64(p[0]|std::uint32_t(p[1])<<8|std::uint32_t(p[2])<<16|std::uint32_t(p[3])<<24));}return a;}
SurfaceBackend::Palette palette(){SurfaceBackend::Palette p{};for(unsigned i=0;i<256;++i){const auto n=i==0x82?0x81:i;p[i]={std::uint8_t(n*17),std::uint8_t(n*29+3),std::uint8_t(n*43+9)};}return p;}
QJsonObject execute(const QJsonObject& c,SurfaceBackend& b){const auto f=unsigned(c["format"].toInt()),kind=unsigned(c["kind"].toInt());const auto pitch=c["pitch"].toInt();const bool imported=c["import"].toBool();auto sr=rows(f,pitch),dr=rows(f,pitch);const auto src=image(f,false),dst=image(f,kind<5);packPixelRows(src,formats[f],sr);packPixelRows(dst,formats[f],dr);const auto originals=sr.bytes,originald=dr.bytes;
 auto s=imported?b.createRows(sr,formats[f]):b.create(src,formats[f]);auto d=kind>=5?s:imported?b.createRows(dr,formats[f]):b.create(dst,formats[f]);
 check(sr.bytes==originals&&dr.bytes==originald);if(f==0){const auto p=b.createPalette(palette());b.bindPaletteObject(s,p);if(d!=s)b.bindPaletteObject(d,p);check(b.releasePalette(p)==0);}
 const auto mode=c["key_mode"].toInt();if(mode!=2){b.setSourceKey(s,key(f));if(mode==1)b.setSourceKey(s,key(f)^1);if(mode==3)b.setSourceKey(s,key(f)&mask(f));}else check(!b.sourceKey(s));
 ClipperState clip;const auto modeClip=c["clip"].toInt();if(modeClip){clip.attached=true;if(modeClip==1)clip.regions=std::vector<Rect>{{1,1,6,4}};else if(modeClip==2)clip.regions=std::vector<Rect>{{0,0,3,2},{4,3,7,5}};else if(modeClip==3)clip.regions=std::vector<Rect>{};}b.setClipper(d,clip);
 SurfaceCopyResult result;if(kind==0)result=b.fill(d,Rect{0,0,7,5},0xd3e2f197);else{SurfaceCopyRequest request;request.source=kind>=5?Rect{0,0,6,4}:Rect{0,0,7,5};request.destination=kind>=5?Rect{1,1,7,5}:Rect{0,0,7,5};request.api=kind==3||kind==4?SurfaceCopyApi::BltFast:SurfaceCopyApi::Blt;request.flags=kind==2||kind==6?0x8000:kind==4?1:0;result=b.copy(s,d,request);}
 sr.bytes.assign(256,0xcd);dr.bytes.assign(256,0xcd);b.readRows(s,sr);b.readRows(d,dr);
 QJsonObject out{{"id",c["id"]},{"variant",c["variant"]},{"hresult",qint64(result.hresult)},{"source",words(b.read(s).pixels)},{"destination",words(b.read(d).pixels)},{"source_descriptor",descriptor(b,s)},{"destination_descriptor",descriptor(b,d)},{"source_bytes",QString(QByteArray(reinterpret_cast<const char*>(sr.bytes.data()),256).toHex())},{"destination_bytes",QString(QByteArray(reinterpret_cast<const char*>(dr.bytes.data()),256).toHex())},{"rgba",rgba(b.present(d))}};
 if(d!=s)b.destroy(d);
 b.destroy(s);check(b.stats().surfaces==0&&b.paletteObjects()==0);return out;
}
void guards(SurfaceBackend& b){
 for(unsigned f=0;f<5;++f){const auto format=formats[f];auto good=rows(f,-(7*int(format.bits/8)+4));packPixelRows(image(f,false),format,good);const auto unchanged=good.bytes;
  for(unsigned bad=0;bad<6;++bad){auto r=good;if(bad==0)r.pitch=0;if(bad==1)r.pitch=INT32_MIN;if(bad==2)r.firstRow=SIZE_MAX;if(bad==3)r.firstRow=0;if(bad==4)r.bytes.resize(20);if(bad==5)r.height=2049;refuse([&]{b.createRows(r,format);});check(b.stats().surfaces==0&&r.bytes==(bad==4?std::vector<std::uint8_t>(unchanged.begin(),unchanged.begin()+20):unchanged));}
  auto s=b.createRows(good,format);SurfaceBackend::Descriptor d{};check(b.lock(s,d)==0&&std::int32_t(d[4])==good.pitch);auto patch=rows(f,7*int(format.bits/8));packPixelRows(image(f,true),format,patch);refuse([&]{b.updateRows(s,0,0,patch);});b.writeLockedRows(s,0,0,patch);check(b.lockedPixels(s).pixels==image(f,true).pixels);check(b.unlock(s)==0&&b.read(s).pixels==image(f,true).pixels);
  auto invalid=image(f,false);if(f<4){invalid.pixels.back()=1u<<format.bits;refuse([&]{packPixelRows(invalid,format,good);});check(good.bytes==unchanged);refuse([&]{b.setSourceKey(s,1u<<format.bits);});check(!b.sourceKey(s));}
  auto small=good;small.width=6;refuse([&]{b.readRows(s,small);});check(small.bytes==unchanged);
  const auto alias=b.alias(s);check(alias==s&&b.release(s)==1);b.markLost(alias);check(b.restore(alias)==0);refuse([&]{b.readRows(alias,good);});check(good.bytes==unchanged);b.updateRows(alias,0,0,patch);check(b.read(alias).pixels==image(f,true).pixels);
  const auto back=b.createRows(patch,format);b.attachBackBuffer(alias,back);check(b.flip(alias)==0);check(b.lock(alias,d)==0&&std::int32_t(d[4])==patch.pitch);check(b.unlock(alias)==0);check(b.lock(back,d)==0&&std::int32_t(d[4])==good.pitch);check(b.unlock(back)==0);b.destroy(back);b.destroy(alias);check(b.stats().surfaces==0);
 }
 for(auto format:std::vector<PixelFormat>{{8,{1,0,0}},{16,{0xf800,0x7c0,31}},{16,{0xf800,0x7e0,0}},{24,{0xff000000,0xff00,255}},{32,{0xff0000,0xffff,255}}})refuse([&]{b.create(image(4,false),format);});
 const auto a=b.create(image(1,false),formats[1]),d=b.create(image(2,false),formats[2]);SurfaceCopyRequest request;request.source=Rect{0,0,7,5};request.destination=Rect{0,0,7,5};refuse([&]{b.copy(a,d,request);});refuse([&]{b.attachBackBuffer(a,d);});check(b.read(d).pixels==image(2,false).pixels);b.destroy(a);b.destroy(d);
}
}
int main(int argc,char** argv){QGuiApplication app(argc,argv);try{if(argc!=3)throw std::runtime_error("inputs outputs required");QFile in(argv[1]);if(!in.open(QIODevice::ReadOnly))throw std::runtime_error("inputs");const auto cases=QJsonDocument::fromJson(in.readAll()).array();SurfaceBackend backend;QJsonArray outputs;for(const auto c:cases)outputs.append(execute(c.toObject(),backend));guards(backend);QFile out(argv[2]);if(!out.open(QIODevice::WriteOnly))throw std::runtime_error("outputs");out.write(QJsonDocument(QJsonObject{{"rows",outputs},{"checks",int(checks)},{"refusals",int(refusals)}}).toJson(QJsonDocument::Compact));std::cout<<"format rows="<<outputs.size()<<" checks="<<checks<<" refusals="<<refusals<<'\n';return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
