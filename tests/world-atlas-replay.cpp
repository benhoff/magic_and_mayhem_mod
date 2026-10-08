#include "world_frame.hpp"
#include "scene_history.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <chrono>
#include <iostream>
namespace {
QByteArray read(const char* path){QFile file(QString::fromLocal8Bit(path));if(!file.open(QIODevice::ReadOnly)||file.size()>32*1024*1024)throw std::runtime_error("Replay input unavailable or too large");return file.readAll();}
using Clock=std::chrono::steady_clock;
double ms(Clock::time_point begin){return std::chrono::duration<double,std::milli>(Clock::now()-begin).count();}
QString pixelHash(const mnm::render::Image& image){QByteArray bytes;bytes.reserve(int(image.pixels.size()*2));for(auto word:image.pixels){bytes.append(char(word));bytes.append(char(word>>8));}return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);if(argc<5||argc>20)throw std::invalid_argument("root bindings new-report 1..16 World inputs");
    auto configured=mnm::assets::AssetStore::create(argv[1]);if(auto* error=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(error->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));mnm::assets::ResourceManager resources(store);mnm::legacy::SnapshotResources bindings(store,resources);
    const auto rows=QJsonDocument::fromJson(read(argv[2])).array();if(rows.isEmpty()||rows.size()>256)throw std::invalid_argument("Invalid bindings");
    for(const auto& value:rows){const auto row=value.toObject();bindings.add({mnm::assets::ResourceKind::ui,row.value("id").toString().toStdString()},row.value("sprite").toString().toStdString(),row.value("sha256").toString().toLatin1());}
    int width=0,height=0;std::vector<std::vector<mnm::render::SceneDraw>> queues;
    for(int i=4;i<argc;++i){const auto world=mnm::legacy::decodeWorldFrame(read(argv[i]));if(!width){width=world.width;height=world.height;}if(width!=int(world.width)||height!=int(world.height))throw std::runtime_error("Canvas extent changed");queues.push_back(mnm::legacy::worldDisplay(world,bindings));}
    std::vector<mnm::render::Image> expected;QJsonObject modes;std::uint64_t allPixels=0;
    for(bool atlas:{false,true}){
        mnm::render::GlBlitter renderer;mnm::render::SceneLimits limits;limits.cache.indexedAtlas=atlas;QJsonArray frames;
        std::size_t peakPixels=0,peakSurfaces=0,peakFrames=0;std::uint64_t readbacksBeforeCheck=0;
        {mnm::render::SceneRenderer history(renderer,resources,{width,height,std::vector<std::uint32_t>(std::size_t(width)*height,0)},limits);
            for(unsigned pass=0;pass<2;++pass)for(std::size_t i=0;i<queues.size();++i){
                const auto begin=Clock::now();const auto before=renderer.stats();history.beginFrame(queues[i],i==0?mnm::render::SceneStart::background:mnm::render::SceneStart::retainedCanvas);while(!history.drawNext(32)){}
                const auto frame=history.presentGpu();if(!frame.valid())throw std::runtime_error("Missing normal GPU presentation");
                const auto submitMs=ms(begin);const auto after=renderer.stats();readbacksBeforeCheck+=after.nativeReadbacks-before.nativeReadbacks;
                const auto native=history.read();const auto completeMs=ms(begin);const auto cache=history.cacheStats();
                peakPixels=std::max(peakPixels,std::size_t(cache.pixels));peakSurfaces=std::max(peakSurfaces,cache.surfaces);peakFrames=std::max(peakFrames,cache.frames);
                if(!atlas&&pass==0)expected.push_back(native);else if(native.pixels!=expected.at(i).pixels)throw std::runtime_error("Atlas/retained replay pixels differ");
                allPixels+=native.pixels.size();frames.append(QJsonObject{{"pass",int(pass)},{"frame",qint64(i+1)},{"draws",qint64(queues[i].size())},{"draw_submit_ms",submitMs},{"draw_and_verify_ms",completeMs},{"uploads",qint64(after.uploads-before.uploads)},{"palette_updates",qint64(after.paletteUpdates-before.paletteUpdates)},{"pixel_sha256",pixelHash(native)},{"resident_frames",qint64(cache.frames)},{"resident_pixels",qint64(cache.pixels)}});
            }
        }
        const auto stats=renderer.stats();if(stats.surfaces||readbacksBeforeCheck)throw std::runtime_error("Normal replay read back pixels or leaked surfaces");
        modes[atlas?"atlas":"expanded"]=QJsonObject{{"frames",frames},{"uploads",qint64(stats.uploads)},{"peak_cache_pixels",qint64(peakPixels)},{"peak_cache_surfaces",qint64(peakSurfaces)},{"peak_cache_frames",qint64(peakFrames)},{"normal_native_readbacks",qint64(readbacksBeforeCheck)},{"verification_readbacks",qint64(stats.nativeReadbacks)},{"remaining_surfaces",qint64(stats.surfaces)},{"driver",QString::fromStdString(renderer.driver().renderer)}};
    }
    QJsonObject report{{"success",true},{"pixels_checked",qint64(allPixels)},{"modes",modes},{"original_pixels_used_as_inputs",false},{"native_initial_word",0},{"scope","Identical closed queues replayed twice per path; draw submission and explicit test synchronization timings. Normal presentation has zero native readbacks."}};
    QFile output(QString::fromLocal8Bit(argv[3]));const auto data=QJsonDocument(report).toJson();if(!output.open(QIODevice::NewOnly|QIODevice::WriteOnly)||output.write(data)!=data.size())throw std::runtime_error("Replay report write failed");
    std::cout<<argv[3]<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
