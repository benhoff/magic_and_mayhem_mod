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

namespace {
void save(const QString &name,const QByteArray &bytes){QFile f(name);
 if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly)||f.write(bytes)!=bytes.size())throw std::runtime_error("Physical reference output exists/unwritable");}
QByteArray hash(const QByteArray &b){return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();}
unsigned word(const QByteArray &b,int i){const auto *p=reinterpret_cast<const unsigned char *>(b.constData())+4*i;return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
void append(QByteArray &b,unsigned v){for(int i=0;i<4;++i)b.append(char(v>>(8*i)));}
}
int main(int argc,char **argv){
 QGuiApplication app(argc,argv);if(argc!=3)return 2;const QDir root(QString::fromLocal8Bit(argv[1]));
 Display *display=nullptr;QJsonArray frames;QJsonObject report{{"success",false},{"native_rendering",false},{"all_global_presentation_routes",false}};
 try{
  if(QGuiApplication::platformName()!=QStringLiteral("xcb")||!(display=XOpenDisplay(nullptr)))throw std::runtime_error("Original physical capture requires X11");
  WindowHost host;if(!host.available())throw std::runtime_error("Original window discovery unavailable");
  QElapsedTimer timer;timer.start();unsigned ordinal=1;
  while(!QFile::exists(root.filePath("physical-stop.flag"))){
   if(timer.elapsed()>300000)throw std::runtime_error("Physical peer timed out");
   QFile request(root.filePath(QString("physical-request-%1.bin").arg(ordinal,4,10,QChar('0'))));
   if(!request.open(QIODevice::ReadOnly)||request.size()!=96){app.processEvents();QThread::msleep(1);continue;}
   const auto raw=request.readAll();const auto width=word(raw,12),height=word(raw,13);
   if(word(raw,19)!=ordinal||word(raw,8)!=1||word(raw,2)&0x80000000u||!width||!height||width>2048||height>2048)
    throw std::runtime_error("Invalid original physical request identity");
   QSet<xcb_window_t> candidates;
   for(auto desktop:host.windows()){char *name=nullptr;
    if(XFetchName(display,desktop,&name)&&name){const QString title=QString::fromUtf8(name);XFree(name);
     if(title=="SceneObserver"||title.startsWith("SceneObserver - ")){
      auto child=host.inputWindow(desktop,QSize(int(width),int(height)));if(child)candidates.insert(child);}}}
   if(candidates.size()!=1)throw std::runtime_error("Original physical window is absent/ambiguous");
   const Window target=*candidates.begin();int x,y;Window child;
   if(!XTranslateCoordinates(display,target,DefaultRootWindow(display),0,0,&x,&y,&child))throw std::runtime_error("Original physical placement unavailable");
   XSync(display,False);auto *screen=QGuiApplication::primaryScreen();
   if(!screen||screen->devicePixelRatio()!=1||!screen->geometry().contains(QRect(x,y,int(width),int(height))))
    throw std::runtime_error("Original full plane lies outside physical screen");
   auto grab=[&]{auto image=screen->grabWindow(0,x,y,int(width),int(height)).toImage().convertToFormat(QImage::Format_RGBA8888);
    if(image.size()!=QSize(int(width),int(height)))throw std::runtime_error("Original full physical plane missing");
    QByteArray bytes;bytes.reserve(width*height*4);for(unsigned row=0;row<height;++row)bytes.append(reinterpret_cast<const char *>(image.constScanLine(row)),width*4);return bytes;};
   const auto first=grab();QThread::msleep(5);const auto second=grab();
   if(first!=second)throw std::runtime_error("Original presentation changed inside guarded physical witness");
   const auto name=QString("physical-oracle-%1.rgba").arg(ordinal,4,10,QChar('0'));save(root.filePath(name),first);
   frames.append(QJsonObject{{"ordinal",int(ordinal)},{"attempt",int(word(raw,0))},{"width",int(width)},{"height",int(height)},
    {"source_sequence",int(word(raw,4))},{"queue",int(word(raw,5))},{"canvas",int(word(raw,6))},
    {"request_sha256",QString::fromLatin1(hash(raw))},{"sha256",QString::fromLatin1(hash(first))},
    {"path",name},{"thread",int(word(raw,22))},{"action_record",int(word(raw,23))},{"stable_double_capture",true}});
   QByteArray ack("MNMPACK1",8);append(ack,ordinal);append(ack,word(raw,0));
   save(root.filePath(QString("physical-ack-%1.bin").arg(ordinal,4,10,QChar('0'))),ack);++ordinal;
  }
  if(frames.isEmpty())throw std::runtime_error("No original physical witnesses");
  report["success"]=true;report["original_draw_thread_fenced"]=true;
 }catch(const std::exception &e){report["error"]=QString::fromUtf8(e.what());}
 if(display)XCloseDisplay(display);
 report["frames"]=frames;
 save(QString::fromLocal8Bit(argv[2]),QJsonDocument(report).toJson());
 std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).toStdString()<<'\n';return report["success"].toBool()?0:1;
}
