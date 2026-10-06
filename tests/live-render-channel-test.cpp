#include "live_command_renderer.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QTemporaryDir>
#include <QSurfaceFormat>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
#include <cstring>
namespace {
void check(bool v,const char* text){if(!v)throw std::runtime_error(text);}
void word(QByteArray& b,quint32 v){const auto at=b.size();b.resize(at+4);qToLittleEndian(v,b.data()+at);}
void record(QByteArray& b,unsigned seq,unsigned op,std::initializer_list<unsigned> fields,const QByteArray& data={}){
    word(b,op);word(b,seq);word(b,unsigned(fields.size()*4+data.size()));for(auto v:fields)word(b,v);b+=data;
}
QByteArray fixture(){
    QByteArray b("MNMCMD01");word(b,1);word(b,16);
    record(b,1,1,{1,2,2,16,0xf800,0x7e0,0x1f},QByteArray::fromHex("00f8e0071f00ffff"));
    record(b,2,6,{1});record(b,3,2,{1,0,0,1,1},QByteArray::fromHex("0000"));
    record(b,4,6,{1});record(b,5,7,{1});record(b,6,8,{});return b;
}
struct Writer {
    QFile file;uchar* map=nullptr;unsigned bytes=0;
    explicit Writer(const QString& path):file(path){check(file.open(QIODevice::ReadWrite),"writer open");map=file.map(0,file.size());check(map,"writer map");}
    ~Writer(){file.unmap(map);}
    void set(unsigned at,unsigned value){__atomic_store_n(reinterpret_cast<quint32*>(map+at),qToLittleEndian<quint32>(value),__ATOMIC_RELEASE);}
    void append(const QByteArray& data){std::memcpy(map+64+bytes,data.constData(),data.size());bytes+=data.size();set(20,bytes);}
};
unsigned synthetic(GlViewport& viewport){
    unsigned count=0;const auto bytes=fixture();
    for(unsigned batch:{1u,7u,65536u}){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("ring");check(live.create(path,123,2),"ring create");Writer mapped(path);
        mnm_ring_writer writer{};check(mnm_ring_writer_bind(&writer,mapped.map,MNM_RENDER_COMMANDS_V2_SIZE),"ring claim");unsigned shown=0;
        live.framePresented=[&]{
            const auto image=viewport.grabFramebuffer();const QRgb colors[4]={shown?qRgb(0,0,0):qRgb(255,0,0),qRgb(0,255,0),qRgb(0,0,255),qRgb(255,255,255)};
            for(int y=0;y<64;++y)for(int x=0;x<64;++x){check(image.pixel(x,y)==colors[(y/32)*2+x/32],"ring full framebuffer");}
            ++shown;++count;
        };
        for(qsizetype at=0;at<bytes.size();at+=batch){auto part=bytes.mid(at,batch);check(mnm_ring_write(&writer,part.constData(),part.size())==1,"ring write");check(live.poll(qMin(batch,1048576u)),"ring fragment poll");}
        check(mnm_ring_end(&writer) && live.finishProducer() && shown==2,"ring terminal drain");
        check(live.result()->liveSurfaces==0 && live.result()->stats.nativeReadbacks==0 && live.result()->stats.rgbaReadbacks==0 && viewport.imageUploads()==0,"ring counters");
    }
    for(unsigned batch:{1u,7u,65536u}){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);const auto path=dir.filePath("channel");check(live.create(path,123),"create channel");
        Writer writer(path);writer.set(24,1);unsigned shown=0;
        live.framePresented=[&]{
            const auto image=viewport.grabFramebuffer();check(image.size()==QSize(64,64),"frame dimensions");
            const QRgb colors[4]={shown?qRgb(0,0,0):qRgb(255,0,0),qRgb(0,255,0),qRgb(0,0,255),qRgb(255,255,255)};
            for(int y=0;y<64;++y)for(int x=0;x<64;++x)check(image.pixel(x,y)==colors[(y/32)*2+x/32],"independent full framebuffer");
            ++shown;++count;
        };
        for(qsizetype at=0;at<bytes.size();at+=batch){writer.append(bytes.mid(at,batch));check(live.poll(qMin(batch,1048576u)),"fragment poll");}
        check(!live.ended(),"command END is not producer terminal");writer.set(24,2);check(live.finishProducer() && live.ended(),"producer finish");
        check(shown==2 && live.result()->liveSurfaces==0 && live.result()->stats.nativeReadbacks==0 && live.result()->stats.rgbaReadbacks==0 && viewport.imageUploads()==0,"GPU resources/counters");
    }
    for(unsigned mode=0;mode<9;++mode){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);const auto path=dir.filePath("channel");check(live.create(path,321),"failure create");Writer w(path);w.set(24,1);
        if(mode==0){w.append(bytes.first(bytes.size()-12));w.set(24,2);}
        if(mode==1){w.append(bytes.first(65));w.set(24,2);}
        if(mode==2){w.set(20,MNM_RENDER_COMMANDS_V1_CAPACITY+1);}
        if(mode==3){w.set(24,7);}
        if(mode==4){w.set(16,99);}
        if(mode==5){w.set(36,1);}
        if(mode==6){w.set(24,3);w.set(28,2);}
        if(mode==7){w.append(bytes.first(16));check(live.poll(),"first prefix");w.set(20,0);}
        if(mode==8){w.append(bytes.first(16));check(live.poll(),"interrupted prefix");}
        check(!live.finishProducer() && !live.error().isEmpty(),"failure refused");
        check(!live.result() || live.result()->liveSurfaces==0,"failure resources");check(viewport.frameSize().isEmpty(),"failure clears frame");
    }
    // Terminal publication before reader drain must preserve all bytes.
    {QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("channel");check(live.create(path,987),"terminal create");Writer w(path);w.set(24,1);w.append(bytes);w.set(24,2);for(unsigned i=0;i<unsigned(bytes.size()) && !live.ended();++i)check(live.poll(1),"terminal drain");check(live.ended(),"terminal fully drained");}
    // One Qt poll executes at most 32 decoded records, even with all data present.
    {QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("channel");check(live.create(path,876),"bounded create");Writer w(path);
        QByteArray many("MNMCMD01");word(many,1);word(many,16);record(many,1,1,{1,1,1,16,0xf800,0x7e0,0x1f},QByteArray::fromHex("00f8"));
        for(unsigned seq=2;seq<72;++seq){record(many,seq,6,{1});}
        record(many,72,7,{1});record(many,73,8,{});
        w.set(24,1);w.append(many);w.set(24,2);check(live.poll() && live.result()->commands==32 && !live.ended(),"first 32 records");
        check(live.finishProducer() && live.presentations()==70 && live.result()->commands==73,"drain queued records");}
    // Full-file validation and fragmented framing agree; trailing/huge lengths refuse.
    for(unsigned mode=0;mode<3;++mode){mnm::render::CommandDecoder decoder;auto data=bytes;if(mode==0)data.chop(1);if(mode==1)data+='x';if(mode==2)qToLittleEndian<quint32>(0xffffffff,data.data()+24);bool refused=false;try{for(auto byte:data)decoder.append(QByteArray(1,byte));decoder.finish();}catch(...){refused=true;}check(refused,"decoder malformed refusal");}
    return count;
}
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QApplication app(argc,argv);
    try {
        GlViewport viewport;viewport.setMinimumSize(1,1);viewport.resize(64,64);viewport.show();app.processEvents();check(viewport.ready(),"viewport ready");
        if(argc==1){auto frames=synthetic(viewport);std::printf("{\"success\":true,\"full_frames\":%u,\"failures\":12,\"ordinary_readbacks\":0,\"viewport_uploads\":0}\n",frames);return 0;}
        check(argc==4,"live args: channel active-marker report");viewport.resize(800,600);app.processEvents();LiveCommandRenderer live(viewport);check(live.open(QString::fromLocal8Bit(argv[1])),"open live channel");QJsonArray frames;bool beforeExit=false;QElapsedTimer elapsed;elapsed.start();QTimer timer;
        live.framePresented=[&]{
            QFile marker(QString::fromLocal8Bit(argv[2]));if(marker.open(QIODevice::ReadOnly)){
                bool ok=false;const auto pid=marker.readAll().trimmed().toUInt(&ok);QFile stat(QString("/proc/%1/stat").arg(pid));
                if(ok && stat.open(QIODevice::ReadOnly)){const auto data=stat.readAll();const auto at=data.lastIndexOf(')')+2;
                    beforeExit|=at>=2 && at<data.size() && data[at]!='Z' && data[at]!='X';}
            }
            const auto image=viewport.grabFramebuffer();
            // 800x600 fixture fits one native pixel per display pixel; smaller
            // fixtures sample every native pixel center within the letterbox.
            const auto size=viewport.frameSize();const auto rect=viewport.imageRect();QByteArray pixels(size.width()*size.height()*4,0);
            for(int y=0;y<size.height();++y)for(int x=0;x<size.width();++x){
                const auto color=image.pixel(int(rect.left()+(x+0.5)*rect.width()/size.width()),int(rect.top()+(y+0.5)*rect.height()/size.height()));
                qToLittleEndian<quint32>(color,pixels.data()+(y*size.width()+x)*4);
            }
            frames.append(QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex()));
        };
        QObject::connect(&timer,&QTimer::timeout,[&]{
            if(!live.poll(65536) || live.ended() || elapsed.elapsed()>30000){
                timer.stop();QJsonObject report{{"success",live.ended()},{"error",live.error()},{"frames",frames},{"before_producer_exit",beforeExit},{"presentations",int(live.presentations())},{"viewport_uploads",int(viewport.imageUploads())}};
                if(live.result()){report["native_readbacks"]=int(live.result()->stats.nativeReadbacks);report["rgba_readbacks"]=int(live.result()->stats.rgbaReadbacks);report["live_surfaces"]=int(live.result()->liveSurfaces);}
                QFile out(QString::fromLocal8Bit(argv[3]));check(out.open(QIODevice::WriteOnly),"report write");out.write(QJsonDocument(report).toJson());app.exit(live.ended()?0:8);
            }
        });QFile ready(QString::fromLocal8Bit(argv[3])+".ready");check(ready.open(QIODevice::WriteOnly),"ready marker");ready.close();timer.start(1);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"live channel test failed: %s\n",e.what());return 1;}
}
