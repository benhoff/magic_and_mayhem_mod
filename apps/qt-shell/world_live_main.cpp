#include "live_world_session.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <QJsonDocument>
#include <QTimer>
#include <QElapsedTimer>
#include <QSurfaceFormat>
#include <QLabel>
#include <QVBoxLayout>
#include <iostream>
int main(int argc,char** argv)try{
    QApplication app(argc,argv);QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QCommandLineParser p;p.addHelpOption();p.addOptions({{"root","Native installed asset root.","path"},{"channel","Existing fresh owned World channel.","path"},
        {"frames","Stop after this many presented frames (zero: interactive).","count","0"},{"report","New diagnostic report path.","path"},
        {"timeout","Wait timeout in seconds (zero: interactive).","seconds","0"},{"diagnostics","Retain first/mismatching verification inputs and pixels outside the asset root.","directory"}});p.process(app);
    if(!p.isSet("root")||!p.isSet("channel"))throw std::invalid_argument("Specify --root and --channel");
    bool ok=false;const auto frames=p.value("frames").toUInt(&ok);if(!ok||frames>256)throw std::invalid_argument("Invalid frame bound");
    const auto seconds=p.value("timeout").toUInt(&ok);if(!ok||seconds>600)throw std::invalid_argument("Invalid timeout bound");
    QWidget window;window.setWindowTitle("Magic & Mayhem — Native World (shadow)");
    QVBoxLayout layout(&window);layout.setContentsMargins(0,0,0,0);
    GlViewport viewport;QLabel status;status.setContentsMargins(8,4,8,4);status.setWordWrap(true);
    layout.addWidget(&viewport,1);layout.addWidget(&status);
    LiveWorldSession session(viewport,p.value("channel"),p.value("root"),p.value("diagnostics"));status.setText(session.status());window.resize(800,630);window.show();QElapsedTimer elapsed;elapsed.start();
    QTimer heartbeat;heartbeat.setInterval(5);quint64 heartbeats=0;qint64 lastHeartbeat=0,maxHeartbeatGap=0;
    QObject::connect(&heartbeat,&QTimer::timeout,[&]{const auto now=elapsed.elapsed();maxHeartbeatGap=std::max(maxHeartbeatGap,now-lastHeartbeat);lastHeartbeat=now;++heartbeats;});heartbeat.start();
    QTimer timer;timer.setInterval(16);int code=0;quint64 timerTicks=0;
    QObject::connect(&timer,&QTimer::timeout,[&]{++timerTicks;
        const auto running=session.poll();const auto interval=session.hasPendingFrame()?1:16;if(timer.interval()!=interval)timer.setInterval(interval);if(status.text()!=session.status())status.setText(session.status());
        if(!running){code=2;app.quit();}
        else if(frames&&session.presentations()>=frames)app.quit();
        else if(session.ended())app.quit();
        else if(seconds&&elapsed.elapsed()>qint64(seconds)*1000){code=3;app.quit();}
    });timer.start();app.exec();timer.stop();heartbeat.stop();session.close();auto report=session.report();report["timer_ticks"]=qint64(timerTicks);report["gui_heartbeats"]=qint64(heartbeats);report["gui_max_heartbeat_gap_ms"]=maxHeartbeatGap;report["elapsed_ms"]=elapsed.elapsed();report["exit_code"]=code;
    if(frames&&session.presentations()<frames){if(!code)code=3;report["success"]=false;report["exit_code"]=code;}
    if(p.isSet("report")){QFile out(p.value("report"));const auto bytes=QJsonDocument(report).toJson();if(!out.open(QIODevice::NewOnly|QIODevice::WriteOnly)||out.write(bytes)!=bytes.size()||!out.flush())throw std::runtime_error("Cannot save new World report");}
    std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).constData()<<'\n';return code;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}
