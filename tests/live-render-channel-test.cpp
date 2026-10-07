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
quint64 sustainedBytes;unsigned sustainedCommands,sustainedRetries;
unsigned coalescedPresents,coalescedPaints,coalescedPolls;
struct PaintCounter:QObject {
    unsigned paints=0;
    bool eventFilter(QObject*,QEvent* event) override{if(event->type()==QEvent::Paint)++paints;return false;}
};
struct InputProbe:QObject {
    bool delivered=false;
    bool event(QEvent* event) override{if(event->type()==QEvent::User){delivered=true;return true;}return QObject::event(event);}
};
void coalesced(GlViewport& viewport){
    QTemporaryDir dir;LiveCommandRenderer live(viewport);const auto path=dir.filePath("burst");
    check(live.create(path,228,2),"burst create");Writer mapped(path);mnm_ring_writer writer{};
    check(mnm_ring_writer_bind(&writer,mapped.map,MNM_RENDER_COMMANDS_V2_SIZE),"burst claim");
    QByteArray data("MNMCMD01");word(data,1);word(data,16);unsigned seq=0,shown=0;
    record(data,++seq,1,{1,2,2,16,0xf800,0x7e0,0x1f},QByteArray(8,0));
    for(unsigned i=0;i<70;++i){
        record(data,++seq,2,{1,0,0,2,2},QByteArray::fromHex(i%2?"00f800f800f800f8":"1f001f001f001f00"));
        record(data,++seq,6,{1});
    }
    record(data,++seq,7,{1});record(data,++seq,8,{});
    check(mnm_ring_write(&writer,data.data(),data.size())==1 && mnm_ring_end(&writer),"burst publish");
    live.framePresented=[&]{++shown;};
    PaintCounter counter;viewport.installEventFilter(&counter);InputProbe input;
    unsigned polls=0,paints=0;
    while(!live.ended()){
        const auto before=shown;counter.paints=0;
        check(live.poll(65536),"burst poll");++polls;paints+=counter.paints;
        if(counter.paints)std::fprintf(stderr,"Burst poll %u: %u presents, %u synchronous Paint events\n",polls,shown-before,counter.paints);
        check(!counter.paints,"Historical PRESENTs blocked the Qt event loop with a synchronous repaint");
        if(polls==1){
            check(shown>1 && live.hasPendingCommands(),"Burst did not retain a bounded backlog");
            QApplication::postEvent(&input,new QEvent(QEvent::User));
        }
        QApplication::processEvents();
        check(input.delivered,"Input event starved between bounded command polls");
        check(polls<=10,"Burst failed to drain");
    }
    viewport.removeEventFilter(&counter);
    check(!live.hasPendingCommands() && shown==70 && live.result()->commands==seq,"Burst skipped commands or presentations");
    const auto image=viewport.grabFramebuffer();
    for(int y=0;y<64;++y)for(int x=0;x<64;++x)check(image.pixel(x,y)==qRgb(255,0,0),"Burst did not retain latest complete frame after DELETE/END");
    check(!live.result()->liveSurfaces && !live.result()->stats.nativeReadbacks && !live.result()->stats.rgbaReadbacks && !viewport.imageUploads(),"Burst ownership/readback regression");
    coalescedPresents=shown;coalescedPaints=paints;coalescedPolls=polls;
}
unsigned sustained(GlViewport& viewport){
    QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("sustained");
    check(live.create(path,222,2),"sustained create");Writer mapped(path);mnm_ring_writer writer{};
    check(mnm_ring_writer_bind(&writer,mapped.map,MNM_RENDER_COMMANDS_V2_SIZE),"sustained claim");
    quint64 total=0;unsigned full=0,seq=0,shown=0;QRgb expected=qRgb(255,0,0);
    live.framePresented=[&]{auto image=viewport.grabFramebuffer();for(int y=0;y<64;++y)for(int x=0;x<64;++x)check(image.pixel(x,y)==expected,"sustained independent framebuffer");++shown;};
    const auto send=[&](const QByteArray& bytes){
        total+=bytes.size();for(qsizetype at=0;at<bytes.size();){auto part=bytes.mid(at,65536);int status=mnm_ring_write(&writer,part.data(),part.size());
            check(status>=0,"sustained write");if(!status){++full;check(live.poll(65536),"sustained backpressure");continue;}at+=part.size();}
        while(live.result() && live.result()->commands<seq)check(live.poll(65536),"sustained record drain");
    };
    QByteArray header("MNMCMD01");word(header,1);word(header,16);send(header);
    QByteArray pixels(512*512*4,0);for(qsizetype i=0;i<pixels.size();i+=4)qToLittleEndian<quint32>(0xff0000,pixels.data()+i);
    QByteArray create;record(create,++seq,1,{1,512,512,32,0xff0000,0xff00,0xff},pixels);send(create);
    // Establish consumer before record-drain loop; the first poll may only frame.
    while(!live.result() || live.result()->commands<seq)check(live.poll(65536),"sustained create drain");
    for(unsigned frame=1;frame<=70;++frame){
        const quint32 color=frame%3==0?0xff0000:frame%3==1?0x00ff00:0x0000ff;
        expected=qRgb((color>>16)&255,(color>>8)&255,color&255);
        for(qsizetype i=0;i<pixels.size();i+=4)qToLittleEndian(color,pixels.data()+i);
        QByteArray update;record(update,++seq,2,{1,0,0,512,512},pixels);send(update);
        if(frame%20==0 || frame==70){QByteArray present;record(present,++seq,6,{1});send(present);}
    }
    for(unsigned i=0;i<5000;++i){QByteArray update;record(update,++seq,2,{1,0,0,1,1},pixels.first(4));send(update);}
    QByteArray end;record(end,++seq,7,{1});record(end,++seq,8,{});send(end);
    check(mnm_ring_end(&writer) && live.finishProducer(),"sustained END");
    check(total>quint64(mnm::render::maxCommandBytes) && seq>4096 && full && shown==4,"sustained exceeds old limits");
    check(live.result()->liveSurfaces==0 && live.result()->stats.nativeReadbacks==0 && live.result()->stats.rgbaReadbacks==0 && viewport.imageUploads()==0,"sustained cleanup and counters");
    sustainedBytes=total;sustainedCommands=seq;sustainedRetries=full;
    // Streaming lifetime identity remains bounded: IDs must increase, not reuse.
    {mnm::render::CommandDecoder decoder(mnm::render::CommandStreamMode::Streaming);QByteArray b=header;record(b,1,1,{2,1,1,16,0xf800,0x7e0,0x1f},QByteArray::fromHex("00f8"));record(b,2,7,{2});record(b,3,1,{1,1,1,16,0xf800,0x7e0,0x1f},QByteArray::fromHex("00f8"));bool refused=false;try{decoder.append(b);}catch(...){refused=true;}check(refused,"streaming monotonic identity");}
    {mnm::render::CommandDecoder decoder(mnm::render::CommandStreamMode::Streaming);bool refused=false;try{decoder.append(QByteArray(65537,0));}catch(...){refused=true;}check(refused,"streaming fragment budget");}
    return shown;
}
unsigned synthetic(GlViewport& viewport){
    unsigned count=0;const auto bytes=fixture();
    // Preserve producer refusal reasons without misclassifying a malformed ring.
    for(unsigned reason:{MNM_RENDER_COMMANDS_V2_REASON_GAP,MNM_RENDER_COMMANDS_V2_REASON_OVERFLOW,MNM_RENDER_COMMANDS_V2_REASON_CANCELLED,MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED,MNM_RENDER_COMMANDS_V2_REASON_INVALID,99u}){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("producer-refusal");
        check(live.create(path,456,2),"refusal create");Writer mapped(path);mnm_ring_writer writer{};
        check(mnm_ring_writer_bind(&writer,mapped.map,MNM_RENDER_COMMANDS_V2_SIZE),"refusal claim");
        mnm_ring_fail(&writer,reason);check(!live.poll(),"producer failure accepted");
        check(live.error().contains("Native command producer refused session 456") && live.error().contains(QString("reason %1").arg(reason)),"producer failure lost identity/reason");
        if(reason==MNM_RENDER_COMMANDS_V2_REASON_GAP)check(live.error().contains("GAP"),"GAP diagnostic missing");
        if(reason==99)check(live.error().contains("UNKNOWN"),"unknown reason misclassified");
        check(mnm_ring_load(writer.map+8)==1 && viewport.frameSize().isEmpty(),"refusal did not cancel/clear");
    }
    {QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("invalid-refusal");check(live.create(path,456,2),"invalid refusal create");Writer mapped(path);mapped.set(16,999);mapped.set(24,3);mapped.set(28,MNM_RENDER_COMMANDS_V2_REASON_GAP);
        check(!live.poll() && !live.error().contains("GAP"),"invalid identity trusted producer reason");}
    // A complete checkpoint record exceeds one ring fragment. One GUI poll
    // must honor the requested byte budget while preserving the fragment cap.
    for(unsigned budget:{65536u,1048576u}){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);auto path=dir.filePath("checkpoint-budget");check(live.create(path,812,2),"checkpoint budget create");Writer mapped(path);mnm_ring_writer writer{};check(mnm_ring_writer_bind(&writer,mapped.map,MNM_RENDER_COMMANDS_V2_SIZE),"checkpoint budget claim");
        QByteArray large("MNMCMD01");word(large,1);word(large,16);
        QByteArray pixels(256*128*4,0);for(qsizetype i=0;i<pixels.size();i+=4)qToLittleEndian<quint32>(0xff0000,pixels.data()+i);
        record(large,1,1,{1,256,128,32,0xff0000,0xff00,0xff},pixels);record(large,2,6,{1});record(large,3,7,{1});record(large,4,8,{});
        for(qsizetype at=0;at<large.size();at+=65536){auto part=large.mid(at,65536);check(mnm_ring_write(&writer,part.data(),part.size())==1,"checkpoint budget write");}
        check(mnm_ring_end(&writer),"checkpoint budget END");unsigned shown=0;
        live.framePresented=[&]{auto image=viewport.grabFramebuffer();auto rect=viewport.imageRect();for(int y=0;y<128;++y)for(int x=0;x<256;++x)check(image.pixel(int(rect.left()+(x+.5)*rect.width()/256),int(rect.top()+(y+.5)*rect.height()/128))==qRgb(255,0,0),"checkpoint budget independent pixels");++shown;};
        check(live.poll(budget),"checkpoint budget poll");check(mnm_ring_load(writer.map+9)==(budget==65536?65536u:quint32(large.size())),"checkpoint byte bound and multi-fragment ACK");check(shown==(budget==65536?0u:1u),"checkpoint first-poll presentation");
        check(live.finishProducer() && shown==1 && live.result()->liveSurfaces==0,"checkpoint budget drain and cleanup");
    }
    // Partitioned uploads have a finite256-unit GUI budget, while expensive
    // copies keep the32-command limit. ACK may precede decoded submission.
    for(unsigned small:{0u,1u}){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);const auto path=dir.filePath("partition-budget");
        check(live.create(path,819,2),"partition budget create");Writer mapped(path);mnm_ring_writer writer{};
        check(mnm_ring_writer_bind(&writer,mapped.map,MNM_RENDER_COMMANDS_V2_SIZE),"partition budget claim");
        QByteArray data("MNMCMD01");word(data,1);word(data,16);unsigned seq=0,shown=0;
        QByteArray pixels(64*64*4,0);for(qsizetype i=0;i<pixels.size();i+=4)qToLittleEndian<quint32>(0xff0000,pixels.data()+i);
        record(data,++seq,1,{1,64,64,32,0xff0000,0xff00,0xff},pixels);
        if(!small)record(data,++seq,1,{2,64,64,32,0xff0000,0xff00,0xff},pixels);
        for(unsigned i=0;i<(small?400u:40u);++i){
            if(small)record(data,++seq,2,{1,0,0,1,1},QByteArray::fromHex("00ff0000"));
            else record(data,++seq,3,{1,2,0,0,64,64,0,0,0,0});
        }
        record(data,++seq,6,{small?1u:2u});record(data,++seq,7,{1});
        if(!small)record(data,++seq,7,{2});
        record(data,++seq,8,{});
        check(data.size()<65536 && mnm_ring_write(&writer,data.data(),data.size())==1 && mnm_ring_end(&writer),"partition budget input");
        live.framePresented=[&]{const auto image=viewport.grabFramebuffer();const auto rect=viewport.imageRect();
            for(int y=0;y<64;++y)for(int x=0;x<64;++x){
                const auto expected=small && !x && !y?qRgb(0,255,0):qRgb(255,0,0);
                check(image.pixel(int(rect.left()+(x+.5)*rect.width()/64),int(rect.top()+(y+.5)*rect.height()/64))==expected,"partition budget pixels");
            }++shown;};
        check(live.poll(65536),"partition budget first poll");
        check(live.result()->commands==(small?249u:32u) && !shown && !live.ended(),"partition finite weighted work");
        check(mnm_ring_load(writer.map+9)==unsigned(data.size()),"partition bounded owned-copy ACK");
        check(live.poll(65536) && live.ended() && shown==1 && live.result()->commands==seq && !live.result()->liveSurfaces,"partition ordered eventual drain");
        check(!live.result()->stats.nativeReadbacks && !live.result()->stats.rgbaReadbacks && !viewport.imageUploads(),"partition ordinary GPU counters");++count;
    }
    for(unsigned budget:{0u,MNM_RENDER_COMMANDS_V1_POLL_BYTES+1u}){
        QTemporaryDir dir;LiveCommandRenderer live(viewport);check(live.create(dir.filePath("invalid-budget"),832,2),"invalid budget create");
        check(!live.poll(budget) && live.error()=="Invalid command poll budget" && !live.ended(),"invalid caller budget refusal");
    }
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
        if(argc==1){coalesced(viewport);auto frames=synthetic(viewport)+sustained(viewport);std::printf("{\"success\":true,\"full_frames\":%u,\"checkpoint_budget_cases\":2,\"failures\":14,\"ordinary_readbacks\":0,\"viewport_uploads\":0,\"sustained_bytes\":%llu,\"sustained_commands\":%u,\"sustained_frames\":4,\"full_retries\":%u,\"burst_presents\":%u,\"burst_paints\":%u,\"burst_polls\":%u,\"input_between_polls\":true}\n",frames,static_cast<unsigned long long>(sustainedBytes),sustainedCommands,sustainedRetries,coalescedPresents,coalescedPaints,coalescedPolls);return 0;}
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
                if(live.result()){report["decoded_commands"]=int(live.result()->commands);report["native_readbacks"]=int(live.result()->stats.nativeReadbacks);report["rgba_readbacks"]=int(live.result()->stats.rgbaReadbacks);report["live_surfaces"]=int(live.result()->liveSurfaces);}
                QFile out(QString::fromLocal8Bit(argv[3]));check(out.open(QIODevice::WriteOnly),"report write");out.write(QJsonDocument(report).toJson());app.exit(live.ended()?0:8);
            }
        });QFile ready(QString::fromLocal8Bit(argv[3])+".ready");check(ready.open(QIODevice::WriteOnly),"ready marker");ready.close();timer.start(1);return app.exec();
    }catch(const std::exception& e){std::fprintf(stderr,"live channel test failed: %s\n",e.what());return 1;}
}
