#include "drawing_scene.hpp"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

namespace {
QByteArray readFile(const QString &path){
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)||file.size()>128*1024*1024)
        throw std::runtime_error("Cannot read bounded scene input");
    return file.readAll();
}
QByteArray digest(const QByteArray &bytes){return QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex();}
void save(const QString &path,const QByteArray &bytes){
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly)||file.write(bytes)!=bytes.size())
        throw std::runtime_error("Cannot create scene output");
}
QByteArray pixels(const mnm::render::Image &image){
    QByteArray bytes;bytes.reserve(qsizetype(image.pixels.size()*2));
    for(auto p:image.pixels){bytes.append(char(p));bytes.append(char(p>>8));}return bytes;
}
unsigned number(const QJsonObject &object,const char *name,unsigned minimum=1){
    const auto value=object.value(QLatin1String(name));const auto n=value.toDouble(-1);
    if(n<minimum||n>262144||n!=unsigned(n))throw std::invalid_argument("Invalid scene identity/count");
    return unsigned(n);
}
mnm::legacy::DrawingBoundary boundary(const QJsonObject &object){
    return {number(object,"sequence"),number(object,"queues",0),number(object,"canvas"),number(object,"width"),number(object,"height")};
}
}
int main(int argc,char **argv){
    QGuiApplication app(argc,argv);
    if(argc!=5){std::cerr<<"Usage: mnm-drawing-scene-replay INPUT.bin ASSET_ROOT SCENE.json OUTPUT_DIRECTORY\n";return 2;}
    QJsonObject report{{"success",false},{"original_pixels_used_as_native_inputs",false},{"original_oracles_read",false},{"live_replacement",false}};
    QJsonArray checkpoints,frames;
    bool ownsOutput=false;
    QDir output(QString::fromLocal8Bit(argv[4]));
    try{
        if(output.exists()||!QDir().mkpath(output.absolutePath()))throw std::invalid_argument("Scene output must be new");
        ownsOutput=true;
        const auto input=readFile(QString::fromLocal8Bit(argv[1]));
        const auto stream=mnm::legacy::decodeCanvasProducers({input.begin(),input.end()},false);
        const auto document=QJsonDocument::fromJson(readFile(QString::fromLocal8Bit(argv[3])));
        if(!document.isObject())throw std::invalid_argument("Invalid scene declaration");
        const auto config=document.object();
        const auto schema=config.value("schema").toInt();
        if(schema!=1&&schema!=2)throw std::invalid_argument("Unsupported scene declaration");
        const auto identity=config.value("boundary").toObject();
        const auto finalBoundary=boundary(identity);
        const auto declaredFrames=schema==2?config.value("frames").toArray():QJsonArray{};
        unsigned priorSequence=0,priorQueues=0;
        if(schema==2&&(declaredFrames.isEmpty()||declaredFrames.size()>4096))
            throw std::invalid_argument("Journey requires bounded explicit frame boundaries");
        for(const auto &frame:declaredFrames){
            if(!frame.isObject())throw std::invalid_argument("Invalid journey frame");
            const auto b=boundary(frame.toObject());
            if(b.sequence<=priorSequence||b.queues<priorQueues||b.sequence>finalBoundary.sequence||
               b.width>2048||b.height>2048)throw std::invalid_argument("Unordered journey frame boundaries");
            priorSequence=b.sequence;priorQueues=b.queues;
        }
        if(schema==2&&declaredFrames.last().toObject()!=identity)
            throw std::invalid_argument("Journey final frame differs from terminal boundary");
        if(stream.operations.size()!=finalBoundary.sequence||stream.queues<finalBoundary.queues||stream.operations.empty())
            throw std::invalid_argument("Scene input does not end at declared boundary");
        auto result=mnm::assets::AssetStore::create(std::filesystem::path(argv[2]));
        if(auto *error=std::get_if<mnm::assets::Error>(&result))throw std::runtime_error(error->detail);
        const auto store=std::get<mnm::assets::AssetStore>(std::move(result));
        const auto assets=config.value("assets").toObject();
        auto source=[&](const std::string &name){
            auto opened=store.open(name);
            if(auto *error=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(error->detail);
            auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));
            auto loaded=mnm::assets::readWhole(*file,32*1024*1024);
            if(auto *error=std::get_if<mnm::assets::Error>(&loaded))throw std::runtime_error(error->detail);
            auto bytes=std::get<std::vector<std::uint8_t>>(std::move(loaded));
            const QByteArray data(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));
            const auto expected=assets.value(QString::fromStdString(name)).toString().toLatin1();
            if(expected.size()!=64||digest(data)!=expected)throw std::invalid_argument("Unbound or changed scene source asset");
            return bytes;
        };
        if(assets.isEmpty())throw std::invalid_argument("Scene has no source asset bindings");
        for(auto i=assets.begin();i!=assets.end();++i)source(i.key().toStdString());
        mnm::render::GlBlitter renderer;
        {
            mnm::legacy::DrawingScene scene(renderer,store,source);
            qsizetype nextFrame=0;
            for(const auto &c:stream.operations){
                if(auto checkpoint=scene.apply(c)){
                    const auto bytes=pixels(checkpoint->image);
                    const auto name=QString("checkpoint-%1.565").arg(checkpoint->ordinal,4,10,QChar('0'));
                    // Only explicitly selected outputs are retained; every
                    // checkpoint is still checked for complete native history.
                    if(config.value("retain_checkpoints").toArray().contains(int(checkpoint->ordinal)))save(output.filePath(name),bytes);
                    checkpoints.append(QJsonObject{{"sequence",int(checkpoint->sequence)},{"ordinal",int(checkpoint->ordinal)},{"canvas",int(checkpoint->canvas)},{"width",checkpoint->image.width},{"height",checkpoint->image.height},{"sha256",QString::fromLatin1(digest(bytes))}});
                }
                if(nextFrame<declaredFrames.size()&&scene.sequence()==number(declaredFrames[nextFrame].toObject(),"sequence")){
                    const auto frame=declaredFrames[nextFrame].toObject();
                    scene.publish(boundary(frame));
                    const auto bytes=pixels(scene.read());scene.presentGpu();
                    const auto name=QString("frame-%1.565").arg(++nextFrame,4,10,QChar('0'));
                    save(output.filePath(name),bytes);
                    auto record=frame;record["ordinal"]=int(nextFrame);record["sha256"]=QString::fromLatin1(digest(bytes));
                    frames.append(record);
                }
            }
            if(nextFrame!=declaredFrames.size())throw std::invalid_argument("Journey frame boundary was not reached");
            scene.seal(finalBoundary);
            const auto image=scene.read();const auto bytes=pixels(image);save(output.filePath("scene.565"),bytes);
            scene.presentGpu();
            report["boundary"]=identity;report["scene_sha256"]=QString::fromLatin1(digest(bytes));
            report["world_draws"]=double(scene.worldDraws());report["checkpoints"]=checkpoints;
            report["frames"]=frames;report["frames_published"]=int(scene.frames());
            report["native_gpu_presentation_ready"]=true;
        }
        if(renderer.stats().surfaces)throw std::runtime_error("Scene native surfaces remain after cleanup");
        for(auto i=assets.begin();i!=assets.end();++i)source(i.key().toStdString());
        report["remaining_surfaces"]=double(renderer.stats().surfaces);report["success"]=true;
        report["driver"]=QString::fromStdString(renderer.driver().renderer);
    }catch(const std::exception &error){report["error"]=QString::fromUtf8(error.what());}
    if(ownsOutput)try{save(output.filePath("scene-report.json"),QJsonDocument(report).toJson());}catch(const std::exception &error){std::cerr<<error.what()<<'\n';return 1;}
    std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).toStdString()<<'\n';
    return report.value("success").toBool()?0:1;
}
