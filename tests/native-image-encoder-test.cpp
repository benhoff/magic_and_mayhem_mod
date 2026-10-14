#include "native_image_encoder.hpp"
#include "native_capture_request.hpp"
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
    require(!encoder.trySubmit([]{}).valid(),"Opportunistic task did not skip full queue");
    unsigned ticks=0;QTimer heartbeat;heartbeat.setInterval(1);QObject::connect(&heartbeat,&QTimer::timeout,[&]{++ticks;});heartbeat.start();
    QTimer::singleShot(50,[&]{release.set_value();});QTimer::singleShot(150,&app,&QCoreApplication::quit);app.exec();
    require(ticks>=20&&refused&&blocked.get(),"GUI blocked or encoder queue unbounded");
    require(pixels.get(),"PNG write failed");QImage decoded(path);require(decoded==image,"PNG pixel roundtrip changed");
    bool propagated=false;try{failure.get();}catch(const std::runtime_error&){propagated=true;}
    require(propagated&&encoder.submit([]{return true;}).get(),"Encoder failure broke future work");
    const auto request=directory.filePath("capture-request.json");
    auto write=[&](const QByteArray& bytes){QFile file(request);require(file.open(QIODevice::WriteOnly|QIODevice::Truncate)&&file.write(bytes)==bytes.size(),"Request fixture write");};
    require(encoder.submit([&]{return readNativeCaptureRequest(request);}).get().isEmpty(),"Missing request invented name");
    write("{\"name\":\"combat-00-dialogue\"}");
    require(encoder.submit([&]{return readNativeCaptureRequest(request);}).get()=="combat-00-dialogue"&&QFile::exists(request),"Valid request changed or removed before capture");
    for(const auto& bytes:{QByteArray("{\"name\":\"../escape\"}"),QByteArray("[1,2]"),QByteArray("{bad"),QByteArray("{\"name\":42}"),QByteArray(4097,' '),QByteArray("{\"name\":\"")+QByteArray(80,'a')+"\"}"}){
        write(bytes);require(encoder.submit([&]{return readNativeCaptureRequest(request);}).get().isEmpty()&&!QFile::exists(request),"Malformed/oversized request not removed on worker");
    }
    std::cout<<"{\"success\":true,\"bounded_queue\":true,\"gui_heartbeat_during_blocked_encoder\":true,\"png_pixels_preserved\":true,\"failure_propagated_and_work_continues\":true}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
