// Reference-only acquisition peer. This is never the native rendering backend.
#include "../qt-shell/window_host.hpp"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QPixmap>
#include <QThread>
#include <X11/Xlib.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <time.h>

namespace {
void save(const QString &name,const QByteArray &bytes){QFile f(name);
 if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly)||f.write(bytes)!=bytes.size())throw std::runtime_error("Physical reference output exists/unwritable");}
QByteArray hash(const QByteArray &b){return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();}
unsigned word(const QByteArray &b,int i){const auto *p=reinterpret_cast<const unsigned char *>(b.constData())+4*i;return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
void append(QByteArray &b,unsigned v){for(int i=0;i<4;++i)b.append(char(v>>(8*i)));}
quint64 stamp(){timespec value{};if(clock_gettime(CLOCK_MONOTONIC,&value))throw std::runtime_error("Physical monotonic clock unavailable");return quint64(value.tv_sec)*1000000000u+quint64(value.tv_nsec);}
struct Capture{QByteArray bytes;quint64 begin,end;int x,y,afterX,afterY,width,height;bool stable;};
QJsonObject geometry(const Capture& c){return {{"before",QJsonArray{c.x,c.y,c.width,c.height}},{"after",QJsonArray{c.afterX,c.afterY,c.width,c.height}},{"stable",c.stable}};}
QJsonObject span(const Capture &sample){return {{"begin_ns",QString::number(sample.begin)},{"end_ns",QString::number(sample.end)}};}
}
int main(int argc,char **argv){
 QGuiApplication app(argc,argv);const bool api=(argc==4||argc==6)&&QString::fromLocal8Bit(argv[3])=="--api";
 const bool egl=(argc==4||argc==6)&&QString::fromLocal8Bit(argv[3])=="--egl";if(argc!=3&&!api&&!egl)return 2;
 unsigned transientMiB=256;if(argc==6){bool okay=false;transientMiB=QString::fromLocal8Bit(argv[5]).toUInt(&okay);if(QString::fromLocal8Bit(argv[4])!="--transient-mib"||!okay||!transientMiB||transientMiB>2048)return 2;}const QDir root(QString::fromLocal8Bit(argv[1]));
 Display *display=nullptr;QJsonArray frames,transients;quint64 transientBytes=0;QJsonObject report{{"success",false},{"native_rendering",false},{"all_global_presentation_routes",false},{"source_inputs_bound",false}};
 try{
  if(QGuiApplication::platformName()!=QStringLiteral("xcb")||!(display=XOpenDisplay(nullptr)))throw std::runtime_error("Original physical capture requires X11");
  WindowHost host;if(!host.available())throw std::runtime_error("Original window discovery unavailable");
  QElapsedTimer timer;timer.start();unsigned ordinal=1;
  while(!QFile::exists(root.filePath(egl?"egl-physical-stop.flag":api?"api-physical-stop.flag":"physical-stop.flag"))){
   if(timer.elapsed()>300000)throw std::runtime_error("Physical peer timed out");
   QFile request(root.filePath(QString(egl?"egl-physical-request-%1.bin":api?"api-physical-request-%1.bin":"physical-request-%1.bin").arg(ordinal,api||egl?6:4,10,QChar('0'))));
   if(!request.open(QIODevice::ReadOnly)||request.size()!=(api||egl?128:96)){app.processEvents();QThread::msleep(1);continue;}
   const auto raw=request.readAll();const auto width=word(raw,api||egl?14:12),height=word(raw,api||egl?15:13);
   if((egl?(!((raw.left(8)=="MNMEGL01"&&word(raw,2)==1)||(raw.left(8)=="MNMEGL02"&&word(raw,2)==2))||word(raw,3)!=128||word(raw,4)!=ordinal||word(raw,8)!=11||word(raw,12)!=1):api?(word(raw,12)!=ordinal||word(raw,1)!=1||word(raw,13)!=1||word(raw,8)&0x80000000u):(word(raw,19)!=ordinal||word(raw,8)!=1||word(raw,2)&0x80000000u))||!width||!height||width>2048||height>2048)
    throw std::runtime_error("Invalid original physical request identity");
   QSet<xcb_window_t> candidates;
   for(auto desktop:host.windows()){char *name=nullptr;
    if(XFetchName(display,desktop,&name)&&name){const QString title=QString::fromUtf8(name);XFree(name);
     const QString wanted=api||egl?"DrawingSession":"SceneObserver";
     if(title==wanted||title.startsWith(wanted+" - ")){
      auto child=host.inputWindow(desktop,QSize(int(width),int(height)));if(child)candidates.insert(child);}}}
   if(candidates.size()!=1)throw std::runtime_error("Original physical window is absent/ambiguous");
   const Window target=*candidates.begin();int x,y;Window child;
   if(!XTranslateCoordinates(display,target,DefaultRootWindow(display),0,0,&x,&y,&child))throw std::runtime_error("Original physical placement unavailable");
   XSync(display,False);auto *screen=QGuiApplication::primaryScreen();
   if(!screen||screen->devicePixelRatio()!=1||!screen->geometry().contains(QRect(x,y,int(width),int(height))))
    throw std::runtime_error("Original full plane lies outside physical screen");
   auto grab=[&]{int gx,gy,ax,ay;Window child;XWindowAttributes before{},after{};
    if(!XGetWindowAttributes(display,target,&before)||!XTranslateCoordinates(display,target,DefaultRootWindow(display),0,0,&gx,&gy,&child)||before.width!=int(width)||before.height!=int(height)||!screen->geometry().contains(QRect(gx,gy,int(width),int(height))))throw std::runtime_error("Original capture target geometry unavailable");
    const auto begin=stamp();auto image=screen->grabWindow(0,gx,gy,int(width),int(height)).toImage().convertToFormat(QImage::Format_RGBA8888);
    if(image.size()!=QSize(int(width),int(height)))throw std::runtime_error("Original full physical plane missing");
    QByteArray bytes;bytes.reserve(width*height*4);for(unsigned row=0;row<height;++row)bytes.append(reinterpret_cast<const char *>(image.constScanLine(row)),width*4);const auto end=stamp();if(!XGetWindowAttributes(display,target,&after)||!XTranslateCoordinates(display,target,DefaultRootWindow(display),0,0,&ax,&ay,&child))throw std::runtime_error("Original capture target retired");return Capture{bytes,begin,end,gx,gy,ax,ay,int(width),int(height),gx==ax&&gy==ay&&after.width==before.width&&after.height==before.height};};
   auto first=grab();QThread::msleep(5);auto second=grab();QJsonArray settling;
   for(unsigned sample=1;(api||egl)&&(first.bytes!=second.bytes||!first.stable||!second.stable||first.x!=second.x||first.y!=second.y)&&sample<=32;++sample){
    if(transientBytes+quint64(first.bytes.size())>quint64(transientMiB)*1024*1024)throw std::runtime_error("Physical transient retention budget exhausted");
    const auto path=QString(egl?"egl-physical-transient-%1-%2.rgba":"api-physical-transient-%1-%2.rgba").arg(ordinal,6,10,QChar('0')).arg(sample,2,10,QChar('0'));
    save(root.filePath(path),first.bytes);transientBytes+=quint64(first.bytes.size());
    QJsonObject item{{"ordinal",int(ordinal)},{"sample",int(sample)},{"width",int(width)},{"height",int(height)},
     {"path",path},{"sha256",QString::fromLatin1(hash(first.bytes))},{"capture",span(first)}};settling.append(item);transients.append(item);
    first=second;QThread::msleep(5);second=grab();
   }
   if(first.bytes!=second.bytes||!first.stable||!second.stable||first.x!=second.x||first.y!=second.y)throw std::runtime_error("Original presentation changed inside guarded physical witness");
   const auto name=QString(egl?"egl-physical-oracle-%1.rgba":api?"api-physical-oracle-%1.rgba":"physical-oracle-%1.rgba").arg(ordinal,api||egl?6:4,10,QChar('0'));save(root.filePath(name),first.bytes);
   QJsonObject frame{{"ordinal",int(ordinal)},{"attempt",int(word(raw,0))},{"width",int(width)},{"height",int(height)},
    {"source_sequence",int(word(raw,4))},{"queue",int(word(raw,5))},{"canvas",int(word(raw,6))},
    {"request_sha256",QString::fromLatin1(hash(raw))},{"sha256",QString::fromLatin1(hash(first.bytes))},
    {"path",name},{"thread",int(word(raw,22))},{"action_record",int(word(raw,23))},{"stable_double_capture",true}};
   if(api){frame.remove("attempt");frame.remove("source_sequence");frame.remove("queue");frame.remove("canvas");frame.remove("action_record");
    frame["record"]=int(word(raw,0));frame["entry"]=int(word(raw,29));frame["method"]=int(word(raw,2));frame["object"]=int(word(raw,3));frame["caller"]=int(word(raw,5));frame["thread"]=int(word(raw,6));}
   if(egl){frame.remove("attempt");frame.remove("source_sequence");frame.remove("queue");frame.remove("canvas");frame.remove("action_record");
    frame["pid"]=int(word(raw,5));frame["thread"]=int(word(raw,6));frame["entry"]=int(word(raw,7));frame["method"]=11;frame["display"]=int(word(raw,9));frame["surface"]=int(word(raw,10));frame["alias"]=int(word(raw,11));}
   frame["clock_domain"]="linux_CLOCK_MONOTONIC";frame["first_capture"]=span(first);frame["second_capture"]=span(second);
   frame["x11_target"]=QString::number(quint64(target));frame["root_x"]=first.x;frame["root_y"]=first.y;frame["first_geometry"]=geometry(first);frame["second_geometry"]=geometry(second);
   if(api||egl)frame["settling_samples"]=settling;
   frames.append(frame);
   QByteArray ack(egl?"MNMEACK1":api?"MNMAPI01":"MNMPACK1",8);append(ack,ordinal);append(ack,word(raw,egl?7:0));
   const auto ackName=root.filePath(QString(egl?"egl-physical-ack-%1.bin":api?"api-physical-ack-%1.bin":"physical-ack-%1.bin").arg(ordinal,api||egl?6:4,10,QChar('0')));
   if(egl){save(ackName+".tmp",ack);if(!QFile::rename(ackName+".tmp",ackName))throw std::runtime_error("Physical EGL ACK publish failed");}else save(ackName,ack);++ordinal;
  }
  if(frames.isEmpty())throw std::runtime_error("No original physical witnesses");
  report["success"]=true;if(egl)report["original_egl_caller_fenced"]=true;else report["original_draw_thread_fenced"]=true;
 }catch(const std::exception &e){report["error"]=QString::fromUtf8(e.what());}
 if(display)XCloseDisplay(display);
 report["frames"]=frames;if(api||egl){report["transient_samples"]=transients;report["transient_bytes"]=double(transientBytes);report["transient_byte_budget"]=double(quint64(transientMiB)*1024*1024);report["all_physical_changes_captured"]=false;}report["boundary"]=egl?"observed_post_egl_return_physical":api?"observed_primary_api_physical":"fenced_opaque_primary_point";
 save(QString::fromLocal8Bit(argv[2]),QJsonDocument(report).toJson());
 std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).toStdString()<<'\n';return report["success"].toBool()?0:1;
}
