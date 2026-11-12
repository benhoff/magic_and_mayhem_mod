#include "../compat/legacy/canvas_world.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <chrono>
#include <iostream>
using namespace mnm;
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);if(argc!=3)throw std::invalid_argument("asset-root retained-owned-source");
    auto configured=assets::AssetStore::create(argv[1]);if(auto* e=std::get_if<assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<assets::AssetStore>(std::move(configured));assets::ResourceManager resources(store);legacy::WorldResources bindings(store,resources);
    QFile input(argv[2]);if(!input.open(QIODevice::ReadOnly)||input.size()>16*1024*1024)throw std::runtime_error("Bounded source input unavailable");const auto raw=input.readAll();
    const auto stream=legacy::decodeCanvasProducers(std::vector<std::uint8_t>(raw.begin(),raw.end()),false);
    legacy::WorldFrame frame;bool entered=false;
    for(const auto& c:stream.operations){const auto& r=c.fields;
        if(r[2]==11){if(entered)throw std::runtime_error("Expected exactly one queue");entered=true;frame.sequence=1;frame.width=r[5];frame.height=r[6];frame.stride=r[7];}
        else if(entered&&(r[2]==9||r[2]==26||r[2]==14))frame.draws.push_back(legacy::producerWorldDraw(c));
        else if(entered&&r[2]==5)throw std::runtime_error("Fill expansion is outside this replay input contract");
    }
    if(!entered||frame.draws.empty())throw std::runtime_error("Missing World draws");
    const auto draws=bindings.display(frame);render::Image background{int(frame.width),int(frame.height),{}};
    for(unsigned i=0;i<frame.width*frame.height;++i)background.pixels.push_back((i*1031u+0x1234)&65535);
    render::Image expected;QJsonObject modes;
    for(bool optimized:{false,true}){
        render::GlBlitter renderer;render::SceneLimits limits;limits.cacheColourRectangles=true;limits.batchSmallColourRectangles=optimized;limits.cache.indexedAtlas=true;limits.cache.atlasFrames=512;
        QJsonArray rows;std::uint64_t uploadCalls=0,drawCalls=0;double submit=0,complete=0;
        {render::SceneRenderer scene(renderer,resources,background,limits);
            for(unsigned pass=0;pass<25;++pass){
                const auto before=renderer.stats();const auto begin=std::chrono::steady_clock::now();
                renderer.batch([&]{scene.beginFrame(draws);while(!scene.drawNext(32)){};});
                const auto submitted=std::chrono::steady_clock::now();const auto after=renderer.stats();
                if(after.nativeReadbacks!=before.nativeReadbacks||after.rgbaReadbacks!=before.rgbaReadbacks)throw std::runtime_error("Ordinary replay introduced a readback");
                const auto image=scene.read();const auto done=std::chrono::steady_clock::now();
                if(!optimized&&pass==0)expected=image;else if(image.pixels!=expected.pixels)throw std::runtime_error("Optimized/serial replay pixels differ");
                if(pass>=5){const double a=std::chrono::duration<double,std::milli>(submitted-begin).count(),b=std::chrono::duration<double,std::milli>(done-begin).count();submit+=a;complete+=b;uploadCalls+=after.textureUploadCalls-before.textureUploadCalls;drawCalls+=after.drawCalls-before.drawCalls;
                    rows.append(QJsonObject{{"submit_ms",a},{"complete_ms",b},{"texture_upload_calls",qint64(after.textureUploadCalls-before.textureUploadCalls)},{"draw_calls",qint64(after.drawCalls-before.drawCalls)}});}
            }
        }
        if(renderer.stats().surfaces)throw std::runtime_error("Replay leaked surfaces");
        modes[optimized?"batched":"serial"]=QJsonObject{{"driver",QString::fromStdString(renderer.driver().renderer)},{"samples",rows},{"mean_submit_ms",submit/20},{"mean_complete_ms",complete/20},{"mean_texture_upload_calls",double(uploadCalls)/20},{"mean_draw_calls",double(drawCalls)/20}};
    }
    QByteArray pixels;for(auto p:expected.pixels){pixels.append(char(p));pixels.append(char(p>>8));}
    const QJsonObject report{{"success",true},{"scope","Same retained closed source primitives on an authored RGB565 background,50full-frame comparisons; warm timings include explicit diagnostic readback synchronization. No original pixels/history, simulation, live FPS or physical scanout claim."},{"source_sha256",QString::fromLatin1(QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex())},{"pixel_sha256",QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex())},{"draws",qint64(draws.size())},{"modes",modes}};
    std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
