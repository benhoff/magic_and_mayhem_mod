#include "scene_renderer.hpp"
#include "scene-resource-fixtures.hpp"
#include "surface_copy.hpp"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <iostream>
#include <stdexcept>

using namespace mnm;
namespace {
using Clock=std::chrono::steady_clock;
constexpr int Width=800,Height=600;
constexpr unsigned Seed=0x19985eed;
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
double ms(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
unsigned number(const char* s,unsigned low,unsigned high){
    unsigned n=0;const auto end=s+std::char_traits<char>::length(s);const auto r=std::from_chars(s,end,n);
    require(r.ec==std::errc{}&&r.ptr==end&&n>=low&&n<=high,"Argument outside bounded stress domain");return n;
}
QString hash(const QByteArray& bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
QString imageHash(const render::Image& image){
    QByteArray bytes;bytes.reserve(qsizetype(image.pixels.size()*4+8));
    const auto put=[&](unsigned v){for(unsigned k=0;k<4;++k)bytes.append(char(v>>(k*8)));};
    put(unsigned(image.width));put(unsigned(image.height));for(auto p:image.pixels)put(p);return hash(bytes);
}
render::Image background(int width=Width,int height=Height){
    render::Image image{width,height,{}};unsigned state=Seed;
    for(int y=0;y<height;++y)for(int x=0;x<width;++x){state^=state<<13;state^=state>>17;state^=state<<5;image.pixels.push_back((x+3*y)%17?state&65535:0);}
    return image;
}
render::SpriteColourTable palette(){render::SpriteColourTable p{};for(unsigned i=0;i<256;++i)p[i]=std::uint16_t((i*2017+97*(i&7))^0x52a3);return p;}
assets::Sprite sprite(unsigned width,unsigned height){
    assets::Sprite s;s.storage=assets::SpriteStorage::indexed8;s.palettes.resize(1);
    for(unsigned phase=0;phase<4;++phase){
        assets::SpriteFrame f;f.width=width;f.height=height;f.paletteIndex=0;
        std::vector<std::uint8_t> indices;
        for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){f.opaqueMask.push_back((x/4+2*y+phase)%5!=0);indices.push_back(std::uint8_t(x*17+y*13+phase*19));}
        f.pixels=std::move(indices);s.frames.push_back(std::move(f));
    }
    return s;
}
struct Spec {unsigned width,height,count;};
Spec spec(const std::string& name,unsigned level){
    if(name=="wave")return level==1?Spec{63,31,128}:level==2?Spec{63,31,512}:Spec{160,80,128};
    if(name=="row-distortion")return level==1?Spec{31,15,4}:level==2?Spec{63,31,16}:Spec{127,47,32};
    if(name=="translucency")return level==1?Spec{31,15,64}:level==2?Spec{63,31,192}:Spec{127,63,384};
    if(name=="keyed-overlap")return {8u<<level,8u<<level,1u<<(level-1)};
    throw std::runtime_error("Unknown effect case");
}
void synchronize(render::GlBlitter& renderer){renderer.batch([]{
    auto* gl=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());require(gl,"No native completion context");gl->glFinish();
});}
std::vector<render::SceneDraw> draws(const std::string& name,const Spec& s,unsigned phase,const assets::ResourceId& id){
    std::vector<render::SceneDraw> result;const auto colours=palette();
    for(unsigned i=0;i<s.count;++i){
        render::SceneDraw d{id,(phase+i)%4,int(48+(i*29+phase*3)%(Width-s.width-100)),int(32+(i*17+phase*5)%(Height-s.height-100))};
        d.colours=colours;
        if(name=="wave"){
            d.composite.mode=i%13==0?render::CompositeMode::copy:render::CompositeMode::displace;
            d.composite.rowPeriod=16;d.composite.retainOutsideDisplacement=true;
            for(unsigned row=0;row<16;++row)d.composite.rowOffsets[row]=(row*3+phase+i)%17;
        }else if(name=="row-distortion"){
            render::SpriteRowEffect rows;rows.kind=render::RowEffectKind::sampleDestination;rows.wholeRunClip=true;
            for(unsigned row=0;row<s.height;++row)rows.offsets.push_back(int((row*5+phase+i)%33));
            d.rowEffect=std::move(rows);
        }else{
            d.composite.mode=i%3==0?render::CompositeMode::half:i%3==1?render::CompositeMode::quarterSource:render::CompositeMode::projectedShadow;
            if(d.composite.mode==render::CompositeMode::projectedShadow)d.colours.reset();
        }
        result.push_back(std::move(d));
    }
    return result;
}
QJsonArray inputs(const std::vector<render::SceneDraw>& draws){
    QJsonArray result;for(const auto& d:draws){
        QJsonArray shifts;for(auto n:d.composite.rowOffsets)shifts.append(n);
        QJsonArray rows;if(d.rowEffect)for(auto n:d.rowEffect->offsets)rows.append(n);
        const auto clip=d.viewport.value_or(render::Rect{0,0,Width,Height});
        result.append(QJsonObject{{"x",d.anchorX},{"y",d.anchorY},{"frame",int(d.frame)},{"visible",d.visible},{"clip",d.clip},
            {"viewport",QJsonArray{clip.left,clip.top,clip.right,clip.bottom}},{"has_palette",bool(d.colours)},
            {"mode",int(d.composite.mode)},{"period",int(d.composite.rowPeriod)},{"offsets",shifts},
            {"retain_outside",d.composite.retainOutsideDisplacement},{"whole_opaque_runs",d.wholeOpaqueRunClipping},
            {"row_kind",d.rowEffect?int(d.rowEffect->kind):-1},{"whole_row_runs",d.rowEffect&&d.rowEffect->wholeRunClip},{"destination_rows",rows}});
    }return result;
}
// Independent sequential CPU oracle. Each effect sees the previous effect's
// writes; displacement/row sampling sees one immutable pre-effect image.
render::Image scalar(render::Image image,const assets::Sprite& sprite,const std::vector<render::SceneDraw>& draws){
    const auto colours=palette();
    for(const auto& d:draws){
        const auto& f=sprite.frames.at(d.frame);const auto& indices=std::get<std::vector<std::uint8_t>>(f.pixels);const auto old=image.pixels;
        if(d.composite.mode==render::CompositeMode::projectedShadow){
            for(unsigned y=0;y<f.height;y+=2)for(unsigned x=0;x<f.width;++x){
                if(!f.opaqueMask[y*f.width+x])continue;
                const auto at=std::size_t(d.anchorY+f.height/2+y/2)*image.width+d.anchorX+x+f.height/2-y/2;
                image.pixels.at(at)=(old.at(at)>>1)&0x7bef;
            }continue;
        }
        for(unsigned y=0;y<f.height;++y)for(unsigned x=0;x<f.width;++x){
            const auto source=std::size_t(y)*f.width+x;if(!f.opaqueMask[source])continue;
            const auto at=std::size_t(d.anchorY+int(y))*image.width+d.anchorX+x;const auto s=unsigned(colours[indices[source]]),v=old.at(at);
            auto& word=image.pixels.at(at);
            if(d.rowEffect)word=old.at(at+unsigned(d.rowEffect->offsets[y]));
            else if(d.composite.mode==render::CompositeMode::displace)word=old.at(at+unsigned(d.composite.rowOffsets[(d.anchorY+y)%d.composite.rowPeriod]));
            else if(d.composite.mode==render::CompositeMode::copy)word=s;
            else if(d.composite.mode==render::CompositeMode::half)word=((s>>1)&0x7bef)+((v>>1)&0x7bef);
            else {const auto half=(v>>1)&0x7bef;word=half+((half>>1)&0x7bef)+((s>>2)&(0x7bef>>1)&0x7bef);}
        }
    }return image;
}
std::vector<render::SurfaceCopyRequest> copies(const Spec& s,unsigned phase){
    std::vector<render::SurfaceCopyRequest> result;
    for(unsigned i=0;i<s.count;++i){
        const int x=48+int((phase*3+i*5)%(Width-s.width-100)),y=32+int((phase*5+i*7)%(Height-s.height-100));
        const int dx=phase%2?-1:1;render::SurfaceCopyRequest r;r.source={x,y,x+int(s.width),y+int(s.height)};r.destination={x+dx,y+1,x+dx+int(s.width),y+1+int(s.height)};r.flags=0x01000000;result.push_back(r);
    }return result;
}
QJsonArray inputs(const std::vector<render::SurfaceCopyRequest>& copies){
    QJsonArray result;for(const auto& c:copies)result.append(QJsonObject{{"api",int(c.api)},{"sx",c.source.left},{"sy",c.source.top},{"dx",c.destination.left},{"dy",c.destination.top},{"width",c.source.right-c.source.left},{"height",c.source.bottom-c.source.top},{"flags",double(c.flags)},{"source_busy",c.sourceBusy},{"destination_busy",c.destinationBusy},{"key",0}});return result;
}
render::Image scalar(render::Image image,const std::vector<render::SurfaceCopyRequest>& copies){
    for(const auto& r:copies)for(int y=0;y<r.source.bottom-r.source.top;++y)for(int x=0;x<r.source.right-r.source.left;++x){
        const auto value=image.pixels.at(std::size_t(r.source.top+y)*image.width+r.source.left+x);
        if(value)image.pixels.at(std::size_t(r.destination.top+y)*image.width+r.destination.left+x)=value;
    }return image;
}
QJsonObject counters(const render::RenderStats& a,const render::RenderStats& b){
    return {{"draw_calls",double(b.drawCalls-a.drawCalls)},{"draw_instances",double(b.drawInstances-a.drawInstances)},
        {"texture_upload_calls",double(b.textureUploadCalls-a.textureUploadCalls)},{"texture_upload_bytes",double(b.textureUploadBytes-a.textureUploadBytes)},
        {"snapshot_pixels",double(b.snapshotPixels-a.snapshotPixels)},{"canvas_clones",double(b.nativeSnapshotCopies-a.nativeSnapshotCopies)},
        {"copy_operations",double(b.copies-a.copies)},{"copy_batches",double(b.copyBatches-a.copyBatches)},{"scratch_allocations",double(b.scratchAllocations-a.scratchAllocations)}};
}
QJsonObject guards(){
    render::GlBlitter renderer;const auto base=background(80,80);const auto target=renderer.create(base,render::spriteFormat);
    render::SurfaceCopyRequest bad;bad.source={0,0,65,64};bad.destination={1,1,66,65};bad.flags=0x01000000;
    bool refused=false;const auto before=renderer.stats();
    try{renderer.surfaceCopy(target,target,bad,0);}catch(const std::runtime_error& e){refused=std::string(e.what()).find("pixel budget")!=std::string::npos;}
    require(refused&&renderer.read(target).pixels==base.pixels&&renderer.stats().drawCalls==before.drawCalls,"Keyed4096pixel guard changed pixels/draws");
    render::SurfaceCopyRequest good;good.source={0,0,8,8};good.destination={1,1,9,9};good.flags=0x01000000;
    const auto expected=scalar(base,std::vector<render::SurfaceCopyRequest>{good});
    renderer.surfaceCopy(target,target,good,0);
    const bool unbatched=renderer.read(target).pixels==expected.pixels;
    renderer.update(target,0,0,base);
    renderer.batch([&]{renderer.surfaceCopy(target,target,good,0);});
    const bool batched=renderer.read(target).pixels==expected.pixels;
    renderer.destroy(target);require(!renderer.stats().surfaces,"Guard surface leak");
    return {{"success",unbatched&&batched},{"renderer",QString::fromStdString(renderer.driver().renderer)},{"keyed_pixel_cap",4096},{"oversize_refused_without_mutation",true},{"unbatched_keyed_pixels_match",unbatched},{"batched_keyed_pixels_match",batched}};
}
}
int main(int argc,char** argv){
    QJsonObject report{{"schema",1},{"success",false}};
    try{
        if(argc==2&&std::string(argv[1])=="--describe"){
            QJsonArray cases;for(const auto& name:{"wave","row-distortion","translucency","keyed-overlap"})for(unsigned level=1;level<=3;++level){const auto s=spec(name,level);cases.append(QJsonObject{{"case",name},{"level",int(level)},{"sprite_width",int(s.width)},{"sprite_height",int(s.height)},{"operations",int(s.count)}});}
            report={{"schema",1},{"success",true},{"canvas_width",Width},{"canvas_height",Height},{"seed",double(Seed)},{"cases",cases}};
        }else if(argc==2&&std::string(argv[1])=="guards"){
            QGuiApplication app(argc,argv);report=guards();
        }else{
            require(argc==7,"Expected CASE LEVEL FRAMES REPEATS WARMUP GPU_TIMING, --describe or guards");
            const std::string name=argv[1];const auto level=number(argv[2],1,3),frames=number(argv[3],1,120),repeats=number(argv[4],1,5),warmup=number(argv[5],0,8),gpu=number(argv[6],0,1);const auto s=spec(name,level);
            require((frames+warmup)*repeats<=256,"Total frame budget exceeds256");
            QGuiApplication app(argc,argv);require(!render::GlBlitter::isolatedRecoveryEnabled(),"Effect stress requires the direct native GL owner");
            const auto source=sprite(s.width,s.height);const auto base=background();const auto raw=test::spr(source);
            const auto fixtureHash=hash(QByteArray(reinterpret_cast<const char*>(raw.data()),qsizetype(raw.size()))),baseHash=imageHash(base);
            QByteArray paletteBytes;for(auto word:palette()){paletteBytes.append(char(word));paletteBytes.append(char(word>>8));}
            const auto paletteHash=hash(paletteBytes);
            QJsonArray rows;std::vector<QString> goldenInputs,goldenPixels;
            report.insert("case",QString::fromStdString(name));report.insert("level",int(level));report.insert("frames",int(frames));report.insert("repeats",int(repeats));report.insert("warmup",int(warmup));report.insert("gpu_timing",bool(gpu));
            report.insert("definition",QJsonObject{{"input_schema",2},{"format","RGB565"},{"seed",double(Seed)},{"canvas_width",Width},{"canvas_height",Height},{"sprite_width",int(s.width)},{"sprite_height",int(s.height)},{"operations",int(s.count)},{"fixture_sha256",fixtureHash},{"background_sha256",baseHash},{"palette_sha256",paletteHash}});
            for(unsigned repeat=0;repeat<repeats;++repeat){
                render::GlBlitter renderer;report.insert("renderer",QString::fromStdString(renderer.driver().renderer));
                {
                    test::SceneResources resources;const auto id=resources.add("effect",source);resources.manager.load(id);
                    render::SceneLimits limits;limits.cache.indexedAtlas=true;limits.cache.atlasFrames=512;limits.cacheColourRectangles=true;limits.batchAdditiveSprites=true;limits.maxAdditiveSprites=512;limits.batchAdditiveRectangles=true;limits.maxAdditiveRectangles=512;
                    std::unique_ptr<render::SceneRenderer> scene;render::SurfaceId target=0;
                    if(name=="keyed-overlap")target=renderer.create(base,render::spriteFormat);else scene=std::make_unique<render::SceneRenderer>(renderer,resources.manager,base,limits);
                    for(unsigned frame=0;frame<frames+warmup;++frame){
                        const auto commands=name=="keyed-overlap"?std::vector<render::SceneDraw>{}:draws(name,s,frame,id);const auto copyCommands=name=="keyed-overlap"?copies(s,frame):std::vector<render::SurfaceCopyRequest>{};
                        const auto description=QJsonObject{{"input_schema",2},{"format","RGB565"},{"fixture",fixtureHash},{"background",baseHash},{"palette",paletteHash},{"phase",int(frame)},{"draws",inputs(commands)},{"copies",inputs(copyCommands)}};
                        const auto inputHash=hash(QJsonDocument(description).toJson(QJsonDocument::Compact));
                        if(target)renderer.update(target,0,0,base);
                        synchronize(renderer);
                        const auto before=renderer.stats();double admission=0;const auto start=Clock::now();
                        const auto effect=[&]{
                            if(target){for(const auto& c:copyCommands)require(renderer.surfaceCopy(target,target,c,0).hresult==0,"Keyed copy refused");}
                            else{const auto begun=Clock::now();scene->beginFrame(commands);admission=ms(begun);while(!scene->drawNext(512)){}}
                        };
                        if(gpu)require(renderer.profileBatch(frame+1,effect),"GPU timestamp scope unavailable");else renderer.batch(effect);
                        const auto submit=ms(start);synchronize(renderer);const auto completion=ms(start);const auto after=renderer.stats();
                        require(after.nativeReadbacks==before.nativeReadbacks&&after.rgbaReadbacks==before.rgbaReadbacks,"Ordinary effect delivery read back pixels");
                        QJsonObject row{{"repeat",int(repeat)},{"frame",int(frame)},{"warmup",frame<warmup},{"input_sha256",inputHash},{"submit_ms",submit},{"completion_ms",completion},{"admission_ms",admission},{"counters",counters(before,after)}};
                        if(gpu){const auto timing=renderer.pollGpuTimings();require(timing.size()==1&&timing[0].sequence==frame+1,"GPU timestamp identity differs");row.insert("gpu_timeline_ms",timing[0].timelineMs);}
                        const auto expected=target?scalar(base,copyCommands):scalar(base,source,commands);const auto actual=target?renderer.read(target):scene->read();
                        row.insert("expected_sha256",imageHash(expected));row.insert("pixel_sha256",imageHash(actual));row.insert("pixels_equal",actual.pixels==expected.pixels);rows.append(row);report.insert("samples",rows);
                        require(actual.pixels==expected.pixels,"Native pixels differ from independent sequential CPU oracle");
                        if(!repeat){goldenInputs.push_back(inputHash);goldenPixels.push_back(imageHash(actual));}else require(inputHash==goldenInputs.at(frame)&&imageHash(actual)==goldenPixels.at(frame),"Fresh repeat input/pixels differ");
                    }
                    if(target)renderer.destroy(target);
                }
                require(!renderer.stats().surfaces,"Effect stress surface leak");
            }
            require(goldenInputs.size()<2||goldenInputs[0]!=goldenInputs[1],"Authored effect input did not move");
            report.insert("ordinary_readbacks",0);report.insert("repeat_pixels_equal",true);report.insert("success",true);
            report.insert("scope","Authored moving renderer input only. CPU submission and serial CPU+GPU completion include scene admission/background copy and native effect work; source authoring/I/O/base upload/diagnostic readback/hash excluded. GPU timestamps are opt-in perturbed timeline intervals including idle gaps, not GPU busy time. No original engine/AI/gameplay, live FPS, Qt/HUD/audio or original pixel-equivalence claim.");
        }
    }catch(const std::exception& e){report.insert("error",QString::fromLocal8Bit(e.what()));}
    std::cout<<QJsonDocument(report).toJson(QJsonDocument::Compact).constData()<<'\n';return report["success"].toBool()?0:1;
}
