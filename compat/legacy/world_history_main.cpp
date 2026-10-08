#include "world_frame.hpp"
#include "scene_history.hpp"
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>

namespace {
QByteArray read(const QString& path,qsizetype limit){QFile file(path);if(!file.open(QIODevice::ReadOnly)||file.size()>limit)throw std::runtime_error("History input unavailable or too large");auto bytes=file.readAll();if(file.error()!=QFileDevice::NoError)throw std::runtime_error("History input read failed");return bytes;}
quint32 integer(const QJsonObject& object,const char* key){const auto value=object.value(key);const auto n=value.toDouble(-1);if(!value.isDouble()||n<0||n>4294967295.0||n!=quint32(n))throw std::invalid_argument("History metadata integer outside bounds");return quint32(n);}
bool within(const std::filesystem::path& root,const std::filesystem::path& path){return std::mismatch(root.begin(),root.end(),path.begin(),path.end()).first==root.end();}
QString hash(const QByteArray& bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
void save(const std::filesystem::path& path,const QByteArray& bytes){QFile file(QString::fromStdString(path.string()));if(!file.open(QIODevice::NewOnly|QIODevice::WriteOnly)||file.write(bytes)!=bytes.size()||!file.flush())throw std::runtime_error("History output failed");}
struct Frame { mnm::legacy::WorldFrame world;mnm::render::CanvasStamp stamp;bool reset;QString inputHash; };
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QCommandLineParser parser;parser.addHelpOption();
    parser.addOptions({{"root","Read-only installed asset root.","path"},{"bindings","Pinned native SPR bindings.","path"},
        {"timeline","Bounded contiguous World requests with explicit native initialization.","path"},{"output","New output directory outside assets and timeline inputs.","path"}});parser.process(app);
    for(const auto* option:{"root","bindings","timeline","output"})if(!parser.isSet(option))throw std::invalid_argument("Specify --root --bindings --timeline --output");
    const auto timelinePath=std::filesystem::canonical(parser.value("timeline").toStdString()),inputRoot=timelinePath.parent_path();
    const auto document=QJsonDocument::fromJson(read(QString::fromStdString(timelinePath.string()),1024*1024));
    if(!document.isObject())throw std::invalid_argument("History timeline must be an object");
    const auto object=document.object();const auto background=integer(object,"native_clear_word");
    if(object.size()!=3||integer(object,"version")!=1||background>65535||!object.value("frames").isArray())throw std::invalid_argument("History requires version/native_clear_word/frames");
    const auto rows=object.value("frames").toArray();if(rows.isEmpty()||rows.size()>256)throw std::invalid_argument("History timeline requires 1..256 frames");
    std::vector<Frame> frames;quint64 total=0;QSize size;mnm::render::CanvasStamp previous{};
    // Entire metadata/encoded-frame admission precedes output creation/drawing.
    // No original destination file is an accepted input.
    for(const auto& value:rows){
        if(!value.isObject())throw std::invalid_argument("Invalid history frame row");
        const auto row=value.toObject();
        if(row.size()!=5||!row.value("reset").isBool()||!row.value("snapshot").isString()||!row.value("sha256").isString())throw std::invalid_argument("History frame requires canvas/queue/reset/snapshot/sha256");
        const mnm::render::CanvasStamp stamp{integer(row,"canvas"),integer(row,"queue")};const auto reset=row.value("reset").toBool();
        if(!stamp.canvas||!stamp.sequence||(stamp.canvas==previous.canvas&&stamp.sequence<=previous.sequence)||
            (!reset&&(stamp.canvas!=previous.canvas||stamp.sequence!=previous.sequence+1)))throw std::invalid_argument("History source gap/identity change requires an explicit native reset");
        const std::filesystem::path relative=row.value("snapshot").toString().toStdString();
        if(relative.empty()||relative.is_absolute())throw std::invalid_argument("History snapshot must be relative");
        const auto path=std::filesystem::canonical(inputRoot/relative);if(!within(inputRoot,path))throw std::invalid_argument("History snapshot escapes input root");
        const auto bytes=read(QString::fromStdString(path.string()),32*1024*1024);total+=bytes.size();if(total>256*1024*1024)throw std::invalid_argument("History aggregate input budget exceeded");
        const auto inputHash=hash(bytes);if(row.value("sha256").toString()!=inputHash)throw std::invalid_argument("History request hash changed");
        auto world=mnm::legacy::decodeWorldFrame(bytes);const QSize extent(int(world.width),int(world.height));if(!size.isValid())size=extent;else if(extent!=size)throw std::invalid_argument("History canvas resize requires a new service");
        frames.push_back({std::move(world),stamp,reset,inputHash});previous=stamp;
    }
    auto configured=mnm::assets::AssetStore::create(parser.value("root").toStdString());if(auto* error=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(error->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));mnm::assets::ResourceManager resources(store);mnm::legacy::SnapshotResources bindings(store,resources);
    const auto bindingDocument=QJsonDocument::fromJson(read(parser.value("bindings"),1024*1024));
    if(!bindingDocument.isArray()||bindingDocument.array().isEmpty()||bindingDocument.array().size()>256)throw std::invalid_argument("Bindings must be a bounded nonempty array");
    for(const auto& value:bindingDocument.array()){
        if(!value.isObject())throw std::invalid_argument("Invalid binding row");
        const auto row=value.toObject();
        if(row.size()!=3||!row.value("id").isString()||!row.value("sprite").isString()||!row.value("sha256").isString())throw std::invalid_argument("Binding requires id/sprite/sha256 strings");
        bindings.add({mnm::assets::ResourceKind::ui,row.value("id").toString().toStdString()},row.value("sprite").toString().toStdString(),row.value("sha256").toString().toLatin1());
    }
    std::vector<std::vector<mnm::render::SceneDraw>> draws;for(const auto& frame:frames)draws.push_back(mnm::legacy::worldDisplay(frame.world,bindings));
    const auto assetRoot=std::filesystem::canonical(store.root()),output=std::filesystem::weakly_canonical(std::filesystem::absolute(parser.value("output").toStdString()));
    if(within(assetRoot,output)||within(inputRoot,output)||std::filesystem::exists(output))throw std::invalid_argument("History outputs must be new and outside asset/timeline roots");
    if(!std::filesystem::create_directory(output))throw std::runtime_error("Cannot create history output directory");
    mnm::render::GlBlitter renderer;QJsonArray results;
    {mnm::render::SceneHistory history(renderer,resources,{size.width(),size.height(),std::vector<std::uint32_t>(std::size_t(size.width())*size.height(),background)});
        for(std::size_t i=0;i<frames.size();++i){const auto& frame=frames[i];history.beginFrame(frame.stamp,draws[i],frame.reset);while(!history.drawNext(32)){}
            const auto native=history.read();QByteArray pixels;pixels.reserve(int(native.pixels.size()*2));for(const auto pixel:native.pixels){pixels.append(char(pixel));pixels.append(char(pixel>>8));}
            save(output/(std::to_string(i+1)+".565"),pixels);
            results.append(QJsonObject{{"canvas",qint64(frame.stamp.canvas)},{"queue",qint64(frame.stamp.sequence)},{"reset",frame.reset},{"snapshot_sha256",frame.inputHash},{"pixel_sha256",hash(pixels)},{"pixels",qint64(native.pixels.size())}});
        }
    }
    const auto stats=renderer.stats();if(stats.surfaces)throw std::runtime_error("Native history leaked surfaces");
    const QJsonObject report{{"success",true},{"frames",results},{"native_clear_word",qint64(background)},{"native_readbacks",qint64(stats.nativeReadbacks)},
        {"remaining_surfaces",qint64(stats.surfaces)},{"original_pixels_used_as_native_inputs",false},{"original_initialization_recovered",false},{"live_replacement",false}};
    save(output/"report.json",QJsonDocument(report).toJson());std::cout<<(output/"report.json").string()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
