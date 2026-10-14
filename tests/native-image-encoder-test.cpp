#include "native_image_encoder.hpp"
#include <QCoreApplication>
#include <QImage>
#include <QTemporaryDir>
#include <QTimer>
#include <iostream>
static void require(bool v,const char* reason){if(!v)throw std::runtime_error(reason);}
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture directory");
    NativeImageEncoder encoder;std::promise<void> started,release;auto unlocked=release.get_future();
    auto blocked=encoder.submit([&]{started.set_value();unlocked.wait();return true;});started.get_future().wait();
    QImage image(800,600,QImage::Format_RGB32);image.fill(0xff124578);const auto path=directory.filePath("pixels.png");
    auto pixels=encoder.submit([image,path]{return image.save(path);});
    auto failure=encoder.submit([]()->bool{throw std::runtime_error("Authored encoder failure");});
    bool refused=false;try{encoder.submit([]{});}catch(const std::runtime_error&){refused=true;}
    unsigned ticks=0;QTimer heartbeat;heartbeat.setInterval(1);QObject::connect(&heartbeat,&QTimer::timeout,[&]{++ticks;});heartbeat.start();
    QTimer::singleShot(50,[&]{release.set_value();});QTimer::singleShot(150,&app,&QCoreApplication::quit);app.exec();
    require(ticks>=20&&refused&&blocked.get(),"GUI blocked or encoder queue unbounded");
    require(pixels.get(),"PNG write failed");QImage decoded(path);require(decoded==image,"PNG pixel roundtrip changed");
    bool propagated=false;try{failure.get();}catch(const std::runtime_error&){propagated=true;}
    require(propagated&&encoder.submit([]{return true;}).get(),"Encoder failure broke future work");
    std::cout<<"{\"success\":true,\"bounded_queue\":true,\"gui_heartbeat_during_blocked_encoder\":true,\"png_pixels_preserved\":true,\"failure_propagated_and_work_continues\":true}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
