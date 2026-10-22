// Test/application host: display native-derived frames and retain physical output.
// This host never receives the independent original comparison directory.
#include "../qt-shell/gl_viewport.hpp"
#include "../../assets/asset_file.hpp"
#include "recorded_composition.hpp"
#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLContext>
#include <QPainter>
#include <QScreen>
#include <QThread>
#include <cmath>
#include <iostream>

namespace {
// Owned diagnostic pixels outside the compared game rectangle. Set the ordinal
// only after that GPU frame paints; a same-image marker witnesses X11 delivery
// without looking at the independent original comparison pixels.
constexpr int markerRows=32;
class CaptureWindow final:public QWidget {
public:
    void setOrdinal(unsigned ordinal){const unsigned word=0x4d4e0000u|ordinal;if(word_!=word){word_=word;update();}}
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.fillRect(rect(),Qt::black);
        for(int bit=0;bit<markerRows;++bit)
            p.fillRect(0,height()-markerRows+bit,width(),1,(word_>>bit)&1?Qt::white:Qt::black);
    }
private:
    unsigned word_=0x4d4e0000u;
};
bool markerMatches(const QImage& image,unsigned width,unsigned height,unsigned ordinal){
    if(image.size()!=QSize(int(width),int(height)+markerRows))return false;
    const unsigned word=0x4d4e0000u|ordinal;
    for(int bit=0;bit<markerRows;++bit)for(unsigned x=0;x<width;++x){
        const auto* pixel=image.constScanLine(int(height)+bit)+4*x;
        const unsigned expected=((word>>bit)&1)?255:0;
        if(pixel[0]!=expected||pixel[1]!=expected||pixel[2]!=expected||pixel[3]!=255)return false;
    }
    return true;
}
void fields(const QJsonObject &o, std::initializer_list<const char *> names) {
    if (o.size()!=int(names.size())) throw std::invalid_argument("Missing or extra presentation field");
    for(auto i=o.begin(); i!=o.end(); ++i) {
        bool found=false; for(auto name:names) if(i.key()==QLatin1String(name)) found=true;
        if(!found) throw std::invalid_argument("Unknown presentation field");
    }
}
unsigned number(const QJsonValue &v, unsigned maximum) {
    const double n=v.toDouble(-1);
    if(!v.isDouble() || !std::isfinite(n) || n<1 || n>maximum || n!=std::floor(n))
        throw std::invalid_argument("Invalid presentation number");
    return unsigned(n);
}
QByteArray hash(const QByteArray &b) {return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();}
QByteArray load(const mnm::assets::AssetStore &store, const QString &name, std::uint64_t limit) {
    auto opened=store.open(name.toStdString());
    if(auto *e=std::get_if<mnm::assets::Error>(&opened)) throw std::runtime_error(e->detail);
    auto read=mnm::assets::readWhole(*std::get<std::unique_ptr<mnm::assets::AssetFile>>(opened),limit);
    if(auto *e=std::get_if<mnm::assets::Error>(&read)) throw std::runtime_error(e->detail);
    const auto &b=std::get<std::vector<std::uint8_t>>(read);
    return {reinterpret_cast<const char *>(b.data()), qsizetype(b.size())};
}
void save(const QString &name,const QByteArray &bytes) {
    QFile f(name);
    if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly) || f.write(bytes)!=bytes.size())
        throw std::runtime_error("Cannot create physical presentation output");
}
}
int main(int argc,char **argv) {
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc,argv);
    if(argc!=3 && !(argc==4 && (QByteArray(argv[3])=="--normalized-nearest" || QByteArray(argv[3])=="--recorded-composition"))) return 2;
    const bool recorded=argc==4 && QByteArray(argv[3])=="--recorded-composition";
    const bool normalized=argc==4 && !recorded;
    QJsonObject report{{"schema",1},{"success",false},{"original_oracles_read",false},
                       {"physical_presentation",false},{"original_execution",false},
                       {"display_conversion",normalized?"normalized-nearest":"bit-replicated"}};
    QDir output(QString::fromLocal8Bit(argv[2])); bool owned=false;
    try {
        if(QGuiApplication::platformName()!=QStringLiteral("xcb"))
            throw std::invalid_argument("Physical presentation requires X11");
        auto created=mnm::assets::AssetStore::create(std::filesystem::path(argv[1]));
        if(auto *e=std::get_if<mnm::assets::Error>(&created)) throw std::runtime_error(e->detail);
        const auto store=std::get<mnm::assets::AssetStore>(std::move(created));
        const auto declaration=load(store,"presentation.json",4*1024*1024);
        const auto doc=QJsonDocument::fromJson(declaration);
        if(!doc.isObject()) throw std::invalid_argument("Invalid presentation declaration");
        const auto config=doc.object();
        if(!QOpenGLContext::globalShareContext()) throw std::runtime_error("Qt GPU sharing context unavailable");
        mnm::render::GlBlitter renderer(QOpenGLContext::globalShareContext());
        std::unique_ptr<RecordedComposition> composition;
        if(recorded)composition=std::make_unique<RecordedComposition>(renderer,config,[&](const auto& name,auto limit){return load(store,name,limit);});
        else fields(config,{"schema","frames"});
        if((recorded?number(config.value("schema"),2):number(config.value("schema"),1))<1 || !config.value("frames").isArray())
            throw std::invalid_argument("Invalid presentation schema");
        const auto frames=config.value("frames").toArray();
        if(frames.isEmpty() || frames.size()>8192) throw std::invalid_argument("Invalid presentation frame budget");
        if(recorded)for(unsigned index=1;index<=unsigned(frames.size());++index)if(!QGuiApplication::primaryScreen() || !QGuiApplication::primaryScreen()->geometry().contains(QRect(0,0,int(composition->width(index)),int(composition->height(index))+markerRows)))
            throw std::invalid_argument("Physical screen cannot contain the complete native frame");
        std::uint64_t total=0;
        // Preflight all file identities before opening a window or output directory.
        if(!recorded)for(const auto &v:frames) {
            if(!v.isObject()) throw std::invalid_argument("Invalid presentation frame");
            const auto f=v.toObject(); fields(f,{"path","width","height","sha256"});
            const auto w=number(f.value("width"),2048), h=number(f.value("height"),2048);
            if(!QGuiApplication::primaryScreen() ||
               !QGuiApplication::primaryScreen()->geometry().contains(QRect(0,0,int(w),int(h)+markerRows)))
                throw std::invalid_argument("Physical screen cannot contain the complete native frame");
            const auto b=load(store,f.value("path").toString(),w*h*2);
            total+=b.size();
            if(b.size()!=qsizetype(w*h*2) || total>2ull*1024*1024*1024 ||
               f.value("sha256").toString().toLatin1()!=hash(b))
                throw std::invalid_argument("Presentation source identity or extent changed");
        }
        if(output.exists() || !QDir().mkpath(output.absolutePath()))
            throw std::invalid_argument("Presentation output must be new");
        owned=true; QJsonArray outputs; unsigned paintedFrames=0,ignoredPrePaintSwaps=0;
        {
            CaptureWindow window; window.setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
            GlViewport viewport(&window);
            viewport.setMinimumSize(1,1); viewport.setScaling(GlViewport::Scaling::Sharp);
            unsigned ordinal=0;
            for(const auto &v:frames) {
                const auto f=v.toObject(); const auto w=recorded?composition->width(ordinal+1):number(f.value("width"),2048), h=recorded?composition->height(ordinal+1):number(f.value("height"),2048);
                mnm::render::SurfaceId surface=0;
                if(recorded)surface=composition->next(ordinal+1);
                else{
                const auto bytes=load(store,f.value("path").toString(),w*h*2);
                if(hash(bytes)!=f.value("sha256").toString().toLatin1())
                    throw std::runtime_error("Native-derived frame changed during presentation");
                mnm::render::Image image{int(w),int(h),{}}; image.pixels.reserve(w*h);
                for(qsizetype i=0;i<bytes.size();i+=2) {
                    const unsigned word=std::uint8_t(bytes[i])|(unsigned(std::uint8_t(bytes[i+1]))<<8);
                    // Native-derived words alone determine this fixed display
                    // policy. No original output or measured colour table enters.
                    const unsigned red=((word>>11)*255+15)/31;
                    const unsigned green=(((word>>5)&63)*255+31)/63;
                    const unsigned blue=((word&31)*255+15)/31;
                    image.pixels.push_back(normalized?(red<<16|green<<8|blue):word);
                }
                surface=renderer.create(image,normalized?
                    mnm::render::PixelFormat{32,{0xff0000,0xff00,0xff}}:
                    mnm::render::PixelFormat{16,{0xf800,0x7e0,0x1f}});
                }
                QByteArray nativeBytes;
                if(recorded){const auto image=renderer.read(surface);nativeBytes.reserve(w*h*2);for(auto pixel:image.pixels){nativeBytes.append(char(pixel));nativeBytes.append(char(pixel>>8));}}
                try {
                    bool framePainted=false;
                    viewport.framePaintTiming=[&](bool complete){if(complete){framePainted=true;window.setOrdinal(ordinal+1);}};
                    window.resize(int(w),int(h)+markerRows); window.move(0,0);
                    viewport.resize(int(w),int(h)); viewport.move(0,0);
                    viewport.setGpuFrame(renderer.presentGpu(surface));
                    unsigned swaps=0;
                    auto connection=QObject::connect(&viewport,&GlViewport::frameSwapped,[&]{
                        if(framePainted)++swaps;else ++ignoredPrePaintSwaps;
                    });
                    viewport.show(); window.show(); window.raise(); viewport.update();
                    QElapsedTimer timer; timer.start();
                    while(!swaps && timer.elapsed()<5000) {app.processEvents(); QThread::msleep(1);}
                    QObject::disconnect(connection); viewport.framePaintTiming={};
                    if(!framePainted || !swaps || !viewport.ready() || !viewport.error().isEmpty() ||
                       viewport.devicePixelRatioF()!=1 || viewport.imageRect()!=QRectF(0,0,w,h))
                        throw std::runtime_error(QString("Native viewport admission failed: swaps=%1 ready=%2 error=%3 size=%4x%5 ratio=%6")
                            .arg(swaps).arg(viewport.ready()).arg(viewport.error()).arg(viewport.width())
                            .arg(viewport.height()).arg(viewport.devicePixelRatioF()).toStdString());
                    const auto position=viewport.mapToGlobal(QPoint(0,0));
                    if(!viewport.screen()->geometry().contains(QRect(position,QSize(int(w),int(h)))))
                        throw std::runtime_error("Native viewport lies outside the physical screen");
                    // A swap signal is not physical frame identity. Read the game
                    // and the native ordinal marker in ONE X11 capture, retrying
                    // only on that marker, never on original comparison pixels.
                    QImage capture;
                    do {
                        app.processEvents();
                        capture=viewport.screen()->grabWindow(0,position.x(),position.y(),int(w),int(h)+markerRows)
                            .toImage().convertToFormat(QImage::Format_RGBA8888);
                        if(markerMatches(capture,w,h,ordinal+1))break;
                        QThread::msleep(1);
                    }while(timer.elapsed()<5000);
                    if(!markerMatches(capture,w,h,ordinal+1))throw std::runtime_error("Physical ordinal marker deadline or extent mismatch");
                    const auto physical=capture.copy(0,0,int(w),int(h));
                    QByteArray marker;
                    for(int y=int(h);y<int(h)+markerRows;++y)marker.append(reinterpret_cast<const char*>(capture.constScanLine(y)),w*4);
                    QByteArray rgba; rgba.reserve(w*h*4);
                    for(unsigned y=0;y<h;++y) rgba.append(reinterpret_cast<const char *>(physical.constScanLine(y)),w*4);
                    const auto name=QString("frame-%1.rgba").arg(ordinal++,6,10,QChar('0'));
                    save(output.filePath(name),rgba);
                    QJsonObject item{{"path",name},{"width",int(w)},{"height",int(h)},
                        {"sha256",QString::fromLatin1(hash(rgba))},{"native_source_sha256",f.value("sha256")}};
                    const auto markerName=QString("frame-%1-marker.rgba").arg(ordinal-1,6,10,QChar('0'));
                    save(output.filePath(markerName),marker);
                    item["marker_path"]=markerName;item["marker_sha256"]=QString::fromLatin1(hash(marker));
                    item["physical_ordinal"]=int(ordinal);item["capture_height"]=int(h)+markerRows;
                    item["capture_sha256"]=QString::fromLatin1(hash(rgba+marker));
                    if(recorded){const auto nativeName=QString("frame-%1.565").arg(ordinal-1,6,10,QChar('0'));save(output.filePath(nativeName),nativeBytes);item["index"]=f["index"];item["native_path"]=nativeName;item["native_sha256"]=QString::fromLatin1(hash(nativeBytes));}
                    ++paintedFrames; outputs.append(item);
                    viewport.setGpuFrame({}); if(!recorded)renderer.destroy(surface);
                }catch(...) {viewport.setGpuFrame({});if(!recorded)renderer.destroy(surface);throw;}
            }
            if(viewport.imageUploads()!=0) throw std::runtime_error("GPU viewport used QImage uploads");
            report["qimage_uploads"]=double(viewport.imageUploads());
        }
        if(composition){composition->verify();composition.reset();report["gpu_composition"]=true;report["plan_sha256"]=QString::fromLatin1(hash(declaration));}
        if(renderer.stats().surfaces) throw std::runtime_error("Native presentation surfaces leaked");
        if(load(store,"presentation.json",4*1024*1024)!=declaration)
            throw std::runtime_error("Presentation declaration changed during execution");
        if(!recorded)for(const auto &v:frames) {
            const auto f=v.toObject();
            if(hash(load(store,f.value("path").toString(),2048*2048*2))!=f.value("sha256").toString().toLatin1())
                throw std::runtime_error("Native-derived input changed after presentation");
        }
        report["remaining_surfaces"]=0; report["frames"]=outputs;
        report["driver"]=QString::fromStdString(renderer.driver().renderer);
        report["physical_generation_frames"]=int(paintedFrames);report["diagnostic_marker_rows"]=markerRows;
        report["painted_before_swap_frames"]=int(paintedFrames);report["ignored_prepaint_swaps"]=int(ignoredPrePaintSwaps);
        report["physical_presentation"]=true;report["success"]=true;
    }catch(const std::exception &e) {report["error"]=QString::fromUtf8(e.what());}
    if(owned) save(output.filePath("presentation-report.json"),QJsonDocument(report).toJson());
    std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).toStdString()<<'\n';
    return report.value("success").toBool()?0:1;
}
