// Repeatable original-window XTest actions, with observation rather than assumed outcomes.
#include "../apps/qt-shell/window_host.hpp"
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QThread>
#include <QScreen>
#include <QPixmap>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <csignal>
#include <stdexcept>
namespace {volatile std::sig_atomic_t cancelled=0;void cancel(int){cancelled=1;}}
int main(int argc,char** argv){
 std::signal(SIGTERM,cancel);std::signal(SIGINT,cancel);QGuiApplication app(argc,argv);if(argc!=4)return 2;
 Display* display=XOpenDisplay(nullptr);WindowHost host;QSet<KeyCode> held;QJsonArray observed;bool success=false;QString error;QElapsedTimer clock;clock.start();
 auto pause=[&](int ms){for(int i=0;i<ms;i+=20){if(cancelled)throw std::runtime_error("Cancelled");QThread::msleep(static_cast<unsigned long>(qMin(20,ms-i)));}};
 auto key=[&](KeySym symbol,bool down){auto code=XKeysymToKeycode(display,symbol);if(!code||!XTestFakeKeyEvent(display,code,down,CurrentTime))throw std::runtime_error("Key injection failed");if(down)held.insert(code);else held.remove(code);XSync(display,False);};
 try{
  int a,b,c,d;if(!display||!host.available()||!XTestQueryExtension(display,&a,&b,&c,&d))throw std::runtime_error("Private X11/XTest unavailable");
  QFile schedule(QString::fromLocal8Bit(argv[2]));if(!schedule.open(QIODevice::ReadOnly))throw std::runtime_error("Schedule unavailable");auto actions=QJsonDocument::fromJson(schedule.readAll()).object().value("actions").toArray();if(actions.isEmpty()||actions.size()>64)throw std::runtime_error("Invalid schedule size");
  const QDir root(QString::fromLocal8Bit(argv[1]));
  while(!cancelled&&clock.elapsed()<90000){QFile world(root.filePath("gameplay.bin"));if(world.exists()&&world.size()>=80)break;pause(20);}
  QFile world(root.filePath("gameplay.bin"));if(world.size()<80)throw std::runtime_error("Original World snapshots missing");
  QSet<xcb_window_t> candidates;for(auto desktop:host.windows()){char* name=nullptr;if(XFetchName(display,desktop,&name)&&name){QString title=QString::fromUtf8(name);XFree(name);if(title=="DrawingSession"||title.startsWith("DrawingSession - ")){auto child=host.inputWindow(desktop,QSize(800,600));if(child)candidates.insert(child);}}}
  if(candidates.size()!=1){throw std::runtime_error("Original input child absent/ambiguous");}
  const Window target=*candidates.begin();XSetInputFocus(display,target,RevertToParent,CurrentTime);XSync(display,False);
  int ordinal=0;for(auto value:actions){if(!host.exists(target))throw std::runtime_error("Original input child disappeared");auto action=value.toObject();const auto kind=action.value("kind").toString();auto row=action;row["ordinal"]=++ordinal;row["start_ms"]=clock.elapsed();
   if(kind=="tap"||kind=="hold"){auto name=action.value("key").toString();KeySym symbol=name=="Right"?XK_Right:name=="Left"?XK_Left:name=="period"?XK_period:name=="comma"?XK_comma:name=="Escape"?XK_Escape:NoSymbol;if(symbol==NoSymbol)throw std::runtime_error("Unsupported schedule key");key(symbol,true);pause(kind=="tap"?60:action.value("duration_ms").toInt());key(symbol,false);}
   else if(kind=="move"||kind=="click"){int x=action.value("x").toInt(-1),y=action.value("y").toInt(-1),rx,ry;Window child;if(x<0||y<0||x>=800||y>=600||!XTranslateCoordinates(display,target,DefaultRootWindow(display),x,y,&rx,&ry,&child))throw std::runtime_error("Invalid original client coordinates");if(!XTestFakeMotionEvent(display,DefaultScreen(display),rx,ry,CurrentTime))throw std::runtime_error("Motion failed");XSync(display,False);if(kind=="click"){int button=action.value("button").toInt();if(button!=1&&button!=3)throw std::runtime_error("Unsupported button");XTestFakeButtonEvent(display,button,True,CurrentTime);XSync(display,False);pause(80);XTestFakeButtonEvent(display,button,False,CurrentTime);XSync(display,False);}}
   else if(kind!="wait"){throw std::runtime_error("Unsupported schedule action");}
   pause(action.value("after_ms").toInt(200));row["end_ms"]=clock.elapsed();
   int rx,ry;Window child;if(!XTranslateCoordinates(display,target,DefaultRootWindow(display),0,0,&rx,&ry,&child)){
    if(qEnvironmentVariable("MNM_DRAW_STORAGE_INPUT")!="1"||actions.size()!=1)throw std::runtime_error("Presentation coordinates unavailable");
    row["diagnostic_frame_missing"]=true;row["original_input_window_retired"]=true;row["synchronized_original_frame"]=false;observed.append(row);continue;
   }
   const auto physical=root.filePath("physical");if(!QDir().mkpath(physical))throw std::runtime_error("Presentation directory unavailable");
   const auto name=QString("%1-client-%2.png").arg(QFileInfo(QString::fromLocal8Bit(argv[3])).completeBaseName()).arg(ordinal,4,10,QChar('0'));QFile journal(root.filePath("session.bin"));row["journal_bytes_before_image"]=journal.size();
   auto image=QGuiApplication::primaryScreen()->grabWindow(0,rx,ry,800,600).toImage();QFile pixels(QDir(physical).filePath(name));if(image.size()!=QSize(800,600)||!pixels.open(QIODevice::WriteOnly|QIODevice::NewOnly)||!image.save(&pixels,"PNG"))throw std::runtime_error("Physical viewport acquisition failed");
   row["journal_bytes_after_image"]=journal.size();row["presentation_file"]="physical/"+name;row["synchronized_original_frame"]=false;observed.append(row);
  }success=true;
 }catch(const std::exception& e){error=QString::fromUtf8(e.what());}
 if(display){for(auto code:held)XTestFakeKeyEvent(display,code,False,CurrentTime);if(qEnvironmentVariable("MNM_DRAW_STORAGE_INPUT")!="1"){XTestFakeButtonEvent(display,1,False,CurrentTime);XTestFakeButtonEvent(display,3,False,CurrentTime);}XSync(display,False);XCloseDisplay(display);}
 QFile output(QString::fromLocal8Bit(argv[3]));if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly))return 3;output.write(QJsonDocument(QJsonObject{{"schema",1},{"success",success},{"error",error},{"actions",observed},{"original_memory_written",false},{"semantic_outcomes_assumed",false}}).toJson());return success?0:1;
}
