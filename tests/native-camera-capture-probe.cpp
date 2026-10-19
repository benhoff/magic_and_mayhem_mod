#include "native_capture_request.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QSaveFile>
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

// Interpose the real Qt durable-sync call in this fixture executable only.
// O_TMPFILE can hide the target name, so constrain by the private directory.
static void delayCaptureSync(int fd){
    const char* directory=std::getenv("MNM_TEST_CAPTURE_DIRECTORY");
    if(!directory)return;
    char link[64],path[4096];std::snprintf(link,sizeof(link),"/proc/self/fd/%d",fd);
    auto n=readlink(link,path,sizeof(path)-1);if(n<0)return;path[n]=0;
    if(std::strncmp(path,directory,std::strlen(directory)) || path[std::strlen(directory)]!='/')return;
    static std::atomic<bool> entered{false};if(entered.exchange(true))return;
    QFile marker(QString::fromLocal8Bit(directory)+"/sync-entered");
    if(marker.open(QIODevice::WriteOnly)){marker.write(path,n);marker.close();}
    timespec delay{11,0};while(nanosleep(&delay,&delay)!=0&&errno==EINTR){}
}
extern "C" int fdatasync(int fd) noexcept {delayCaptureSync(fd);return int(syscall(SYS_fdatasync,fd));}
extern "C" int fsync(int fd) noexcept {delayCaptureSync(fd);return int(syscall(SYS_fsync,fd));}

int main(int argc,char** argv){
    QCoreApplication app(argc,argv);if(argc!=3)return 2;
    const bool durable=std::strcmp(argv[1],"durable")==0;
    if(!durable&&std::strcmp(argv[1],"current")!=0)return 2;
    QDir directory(QString::fromLocal8Bit(argv[2]));
    QString name;
    for(unsigned poll=0;poll<500&&name.isEmpty();++poll){
        name=readNativeCaptureRequest(directory.filePath("capture-request.json"));
        if(name.isEmpty())usleep(10000);
    }
    if(name.isEmpty()||!QFile::remove(directory.filePath("capture-request.json")))return 3;
    for(unsigned route=0;route<2;++route){
        QImage image(800,600,QImage::Format_RGB32);
        for(int y=0;y<600;++y)for(int x=0;x<800;++x)
            image.setPixel(x,y,qRgb((x+route*17)&255,(y*3)&255,(x+y+route*29)&255));
        const auto path=directory.filePath(name+(route?"-original.png":"-native.png"));
        bool ok;
        if(durable){QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return 1;QImageWriter writer(&file,"png");writer.setCompression(0);ok=writer.write(image)&&file.commit();}
        else ok=saveNativeCaptureImage(path,image);
        if(!ok)return 1;
    }
    const QJsonObject metadata{{"native_saved",true},{"original_saved",true},{"fallback",false}};
    if(durable){QSaveFile file(directory.filePath(name+".json"));const auto bytes=QJsonDocument(metadata).toJson();return file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size()&&file.commit()?0:1;}
    return saveNativeCaptureMetadata(directory.filePath(name+".json"),metadata)?0:1;
}
