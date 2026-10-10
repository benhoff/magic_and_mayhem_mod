// Bounded XTest fixture, run only inside the capture's disposable Xvfb server.
#include "../apps/qt-shell/window_host.hpp"
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QThread>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <stdexcept>
#include <csignal>
namespace {volatile std::sig_atomic_t cancelled=0;void cancel(int){cancelled=1;}}

int main(int argc,char** argv){
 std::signal(SIGTERM,cancel);std::signal(SIGINT,cancel);
 QGuiApplication app(argc,argv);
 if(argc!=3)return 2;
 QJsonArray actions,titles;bool success=false;QString error;
 Display* display=XOpenDisplay(nullptr);WindowHost host;KeyCode right=0,left=0;
 try{
  if(!display||!host.available())throw std::runtime_error("X11 unavailable");
  int a,b,c,d;if(!XTestQueryExtension(display,&a,&b,&c,&d))throw std::runtime_error("XTest unavailable");
  QElapsedTimer timer;timer.start();xcb_window_t target=0;int step=0;quint64 at=64;unsigned returned=0;
  auto key=[&](KeySym symbol,bool down,int queue){
   const auto code=XKeysymToKeycode(display,symbol);
   if(!code||!XTestFakeKeyEvent(display,code,down?True:False,CurrentTime))throw std::runtime_error("XTest key refused");
   XSync(display,False);actions.append(QJsonObject{{"queue",queue},{"keysym",static_cast<qint64>(symbol)},{"down",down}});
  };
  auto tap=[&](KeySym symbol,int queue){key(symbol,true,queue);QThread::msleep(35);key(symbol,false,queue);};
  const int queues[]={1,3,6,9,12,14,15};
  while(step<7&&!cancelled&&timer.elapsed()<90000){
   QFile file(QDir(QString::fromLocal8Bit(argv[1])).filePath(QStringLiteral("canvas-producers.bin")));
   if(!file.open(QIODevice::ReadOnly)){QThread::msleep(5);continue;}
   const auto header=file.read(64);
   if(header.size()!=64||header.left(8)!="MNMPRO02")throw std::runtime_error("Owned producer header unavailable");
   while(at+96<=quint64(file.size())){
    if(!file.seek(at))throw std::runtime_error("Producer seek refused");
    const auto record=file.read(96);if(record.size()!=96)break;
    const auto size=qFromLittleEndian<quint32>(record.constData());
    if(size<96||size>512u*1024u*1024u)throw std::runtime_error("Invalid bounded producer record");
    if(at+size>quint64(file.size()))break;
    if(qFromLittleEndian<quint32>(record.constData()+8)==12)returned=qFromLittleEndian<quint32>(record.constData()+56);
    at+=size;
    if(returned>=unsigned(queues[step]))break;
   }
   if(returned<unsigned(queues[step])){QThread::msleep(5);continue;}
   if(returned!=unsigned(queues[step]))throw std::runtime_error("Input missed its completed World return");
   if(!target){QSet<xcb_window_t> candidates;for(auto desktop:host.windows()){char* name=nullptr;if(XFetchName(display,desktop,&name)&&name){const auto title=QString::fromUtf8(name);XFree(name);titles.append(title);if(title!=QStringLiteral("SceneObserver")&&!title.startsWith(QStringLiteral("SceneObserver - ")))continue;auto input=host.inputWindow(desktop,QSize(800,600));if(input)candidates.insert(input);}}if(candidates.size()!=1)throw std::runtime_error("Unique game input child absent/ambiguous");target=*candidates.begin();XSetInputFocus(display,target,RevertToParent,CurrentTime);XWarpPointer(display,None,target,0,0,0,0,400,300);XSync(display,False);right=XKeysymToKeycode(display,XK_Right);left=XKeysymToKeycode(display,XK_Left);}
   if(!host.exists(target))throw std::runtime_error("Game input child disappeared");
   switch(step){case 0:key(XK_Right,true,queues[step]);break;case 1:key(XK_Right,false,queues[step]);tap(XK_period,queues[step]);break;case 2:case 3:tap(XK_period,queues[step]);break;case 4:tap(XK_comma,queues[step]);break;case 5:key(XK_Left,true,queues[step]);break;case 6:key(XK_Left,false,queues[step]);break;}
   ++step;
  }
  if(step!=7)throw std::runtime_error("Bounded input schedule incomplete");
  success=true;
 }catch(const std::exception& e){error=QString::fromUtf8(e.what());}
 if(display){if(right)XTestFakeKeyEvent(display,right,False,CurrentTime);if(left)XTestFakeKeyEvent(display,left,False,CurrentTime);XSync(display,False);XCloseDisplay(display);}
 QFile output(QString::fromLocal8Bit(argv[2]));if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly))return 3;
 output.write(QJsonDocument(QJsonObject{{"success",success},{"error",error},{"actions",actions},{"window_titles",titles},{"original_memory_written",false}}).toJson());
 return success?0:1;
}
