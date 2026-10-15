#include "game_process.hpp"
#include "game_restart_action.hpp"
#include "live_menu_session.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QDir>
#include <QTemporaryDir>
#include <QThread>
#include <iostream>
#include <stdexcept>

static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void spin(const std::function<bool()>& ready,int milliseconds=4000){
    QElapsedTimer clock;clock.start();
    while(!ready()&&clock.elapsed()<milliseconds){QCoreApplication::processEvents();QThread::msleep(2);}
    check(ready(),"bounded asynchronous completion");
}
static void script(const QString& path,const QByteArray& body){
    QFile file(path);check(file.open(QIODevice::WriteOnly),"script open");
    check(file.write("#!/usr/bin/python3\n"+body)>0,"script write");file.close();
    check(file.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner),"script permissions");
}
static QByteArray read(const QString& path){QFile file(path);return file.open(QIODevice::ReadOnly)?file.readAll():QByteArray{};}
static bool alive(qint64 pid){
    const auto stat=read(QString("/proc/%1/stat").arg(pid));
    const auto state=stat.mid(stat.lastIndexOf(')')+2,1);
    return !stat.isEmpty()&&state!="Z"&&state!="X";
}
int main(int argc,char** argv) try{
    QApplication app(argc,argv);QTemporaryDir repo;check(repo.isValid(),"temporary repository");
    QWidget window,viewport(&window),otherWindow;QPushButton restart("Hard restart",&window);
    struct InputFilter final:QObject{unsigned keys=0;bool eventFilter(QObject*,QEvent* event) override{if(event->type()==QEvent::ShortcutOverride||event->type()==QEvent::KeyPress){++keys;event->accept();return true;}return false;}} inputFilter;
    viewport.installEventFilter(&inputFilter);unsigned requests=0;
    GameRestartAction action(window,restart,[&]{++requests;});
    const auto shortcut=[&](QWidget& target,QEvent::Type type,bool repeat=false){QKeyEvent key(type,Qt::Key_R,Qt::ControlModifier|Qt::ShiftModifier,QString{},repeat);QApplication::sendEvent(&target,&key);};
    shortcut(viewport,QEvent::KeyPress);check(!requests,"disabled restart shortcut does nothing");
    restart.setEnabled(true);shortcut(viewport,QEvent::ShortcutOverride);shortcut(viewport,QEvent::KeyPress);shortcut(viewport,QEvent::KeyRelease);
    check(requests==1&&!inputFilter.keys,"restart shortcut precedes game input forwarding");
    shortcut(viewport,QEvent::KeyPress,true);check(requests==1,"held shortcut does not repeat");
    window.showFullScreen();restart.hide();shortcut(viewport,QEvent::KeyPress);check(requests==2,"shortcut works with fullscreen toolbar hidden");
    shortcut(otherWindow,QEvent::KeyPress);check(requests==2,"other windows do not restart game");
    restart.show();restart.click();check(requests==3,"toolbar click uses same semantic action");window.hide();
    const auto server=repo.filePath("wineserver"),journal=repo.filePath("server.log");
    script(server,"import os,sys,time\nfrom pathlib import Path\nwith Path(__file__).with_name('server.log').open('a') as f: f.write(sys.argv[1]+' '+os.environ['WINEPREFIX']+'\\n')\ntime.sleep(.08)\n");
    const auto frozen=repo.filePath("frozen.py"),child=repo.filePath("child.pid");
    script(frozen,"import os,signal,time\nfrom pathlib import Path\nsignal.signal(signal.SIGTERM,signal.SIG_IGN)\npid=os.fork()\nif pid: Path(__file__).with_name('child.pid').write_text(str(pid))\nwhile True: time.sleep(1)\n");
    QProcess unrelated;unrelated.start("/usr/bin/sleep",{"30"});check(unrelated.waitForStarted(),"unrelated process started");
    GameProcess process;process.start(frozen);spin([&]{return QFile::exists(child);});
    const auto descendant=read(child).toLongLong();check(alive(descendant),"frozen descendant exists");
    unsigned ticks=0;QTimer heartbeat;heartbeat.setInterval(5);QObject::connect(&heartbeat,&QTimer::timeout,[&]{++ticks;});heartbeat.start();
    bool done=false,success=false;unsigned ordinaryExits=0;
    QObject::connect(&process,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),[&](int,QProcess::ExitStatus){if(!process.hardStopping())++ordinaryExits;});
    const auto prefix=repo.filePath("prefix with spaces");
    process.forceStop(prefix,[&](bool ok,const QString&){success=ok;done=true;},server);
    process.forceStop(prefix,[&](bool,const QString&){throw std::runtime_error("duplicate stop callback");},server);
    spin([&]{return done;});
    check(success&&!process.hardStopping()&&process.state()==QProcess::NotRunning,"shutdown completed before relaunch");
    check(!alive(descendant)&&ticks>3,"frozen child stopped without blocking Qt events");
    check(ordinaryExits==0,"forced exit suppressed until stop callback");
    check(unrelated.state()==QProcess::Running,"unrelated process survived");
    check(read(journal)==("-k "+prefix+"\n-w "+prefix+"\n").toUtf8(),"Wine kill and wait use exact prefix");
    process.start("/usr/bin/sleep",{"30"});spin([&]{return process.state()==QProcess::Running;});
    done=false;process.forceStop({},[&](bool ok,const QString&){success=ok;done=true;},server);spin([&]{return done;});
    check(success&&read(journal).count('\n')==2,"staging cancellation does not stop Wine");
    process.start("/usr/bin/sleep",{"30"});spin([&]{return process.state()==QProcess::Running;});
    done=false;process.forceStop(prefix,[&](bool ok,const QString& error){success=ok;check(!error.isEmpty(),"failed Wine stop explained");done=true;},repo.filePath("missing-wineserver"));
    spin([&]{return done;});check(!success,"failed shutdown refuses relaunch");
    script(server,"import sys\nsys.exit(7)\n");
    done=false;process.forceStop(prefix,[&](bool ok,const QString&){success=ok;done=true;},server);spin([&]{return done;});
    check(!success,"nonzero Wine stop refuses relaunch");
    script(server,"import time\ntime.sleep(30)\n");
    done=false;process.forceStop(prefix,[&](bool ok,const QString& error){success=ok;check(error.contains("timed out"),"shutdown timeout explained");done=true;},server);
    spin([&]{return done;},12000);check(!success,"hung Wine stop refuses relaunch");
    // Exercise LiveMenuSession's staging-exit branch and subsequent reuse.
    check(QDir().mkpath(repo.filePath("tools")),"tools directory");
    check(QDir().mkpath(repo.filePath("evidence")),"evidence directory");
    const auto prepare=repo.filePath("tools/prepare-menu-observer.py");
    script(prepare,"import time\nprint('Evidence directory: staging-must-not-launch',flush=True)\ntime.sleep(30)\n");
    LiveMenuSession session(repo.path());session.preferencesMenusEnabled=false;
    bool staging=false;unsigned launches=0,finishes=0,failures=0;
    session.output=[&](const QString& text){if(text.contains("Evidence directory:"))staging=true;};
    session.launched=[&]{++launches;};session.finished=[&]{++finishes;};session.failed=[&](const QString&){++failures;};
    check(session.start(),"menu staging start");spin([&]{return staging;});
    done=false;session.forceStop([&](bool ok,const QString&){success=ok;done=true;});spin([&]{return done;});
    check(success&&!session.running()&&!launches&&!finishes&&!failures,"cancelled staging cannot launch from finished signal");
    script(prepare,"from pathlib import Path\nprint('Evidence directory: '+str(Path.cwd()/'evidence'),flush=True)\n");
    script(repo.filePath("tools/run-menu-observer.py"),"import time\ntime.sleep(30)\n");
    script(server,"import os,sys\nfrom pathlib import Path\nwith Path(__file__).with_name('server.log').open('a') as f: f.write(sys.argv[1]+' '+os.environ['WINEPREFIX']+'\\n')\n");
    qputenv("PATH",repo.path().toUtf8()+":"+qgetenv("PATH"));session.winePrefix=prefix;
    check(session.start(),"menu relaunch after cancelled staging");spin([&]{return launches==1;});
    done=false;session.forceStop([&](bool ok,const QString&){success=ok;done=true;});spin([&]{return done;});
    check(success&&!session.running()&&!finishes&&!failures,"active menu force-stop bypasses normal quit and exit handlers");
    unrelated.kill();unrelated.waitForFinished();
    std::cout<<"PASS: toolbar and fullscreen shortcut, input-filter priority, frozen process tree, prefix kill/wait, asynchronous heartbeat, repeated stop, reuse, shutdown failures, menu staging and active-session cancellation\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
