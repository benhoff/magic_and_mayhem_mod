#include "world_preparation.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <chrono>
#include <thread>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static void put(QByteArray& b,qsizetype at,std::uint32_t v){for(unsigned i=0;i<4;++i)b[at+i]=char(v>>(i*8));}
static QByteArray wire(){
    QByteArray b(80,0);b.replace(qsizetype(0),qsizetype(8),"MNMWRLD1",qsizetype(8));put(b,8,1);put(b,12,80);put(b,20,1);put(b,24,32);put(b,28,12);put(b,32,32);put(b,44,0x40209ca7);
    struct Request {int x,y,w,h;std::uint32_t r,g,b;render::Rect clip;};
    const render::Rect all{0,0,32,12};
    const Request rows[]={{-3,-2,12,7,12,12,6,all},{4,2,19,8,31,63,31,{3,1,28,11}},{8,3,9,4,65535,65535,65535,all},{-32,0,8,8,1,2,3,all},{31,11,8,8,2,3,4,all},{0,0,32,12,0,0,0,all},{15,4,3,4,65520,65480,65530,all}};
    for(const auto& q:rows){const auto at=b.size();b.append(QByteArray(84,0));put(b,at,84);put(b,at+4,6);put(b,at+8,q.x);put(b,at+12,q.y);put(b,at+16,q.clip.left);put(b,at+20,q.clip.top);put(b,at+24,q.clip.right);put(b,at+28,q.clip.bottom);put(b,at+40,20);put(b,at+52,11);put(b,at+64,q.w);put(b,at+68,q.h);put(b,at+72,q.r);put(b,at+76,q.g);put(b,at+80,q.b);}
    put(b,36,7);put(b,16,b.size());return b;
}
static render::Image background(){render::Image b{32,12,{}};for(unsigned i=0;i<384;++i)b.pixels.push_back((i*7919+0x1234)&65535);return b;}
static render::Image formula(render::Image b,const legacy::WorldFrame& frame){
    for(const auto& d:frame.draws){if(d.colourRectangle){const auto& source=d.colourRectangle->pixels;for(int y=0;y<b.height;++y)for(int x=0;x<b.width;++x){
        if(x<d.x||y<d.y||std::int64_t(x)>=std::int64_t(d.x)+source.width||std::int64_t(y)>=std::int64_t(d.y)+source.height||x<d.clip.left||y<d.clip.top||x>=d.clip.right||y>=d.clip.bottom)continue;
        const auto v=source.pixels[std::size_t(y-d.y)*source.width+x-d.x];auto& dest=b.pixels[std::size_t(y)*b.width+x];
        if(d.composite.mode==render::CompositeMode::copy)dest=v;
        else if(d.composite.mode==render::CompositeMode::half)dest=((v>>1)&0x7bef)+((dest>>1)&0x7bef);
        else if(d.composite.mode==render::CompositeMode::quarterSource){const auto half=(dest>>1)&0x7bef;dest=half+((half>>1)&0x7bef)+((v>>2)&0x39e7);}
        else{const auto half=(v>>1)&0x7bef;dest=half+((half>>1)&0x7bef)+((dest>>2)&0x39e7);}
    }continue;}const auto a=*d.additive;for(int y=0;y<b.height;++y)for(int x=0;x<b.width;++x){
        if(x<d.x||y<d.y||std::int64_t(x)>=std::int64_t(d.x)+a.width||std::int64_t(y)>=std::int64_t(d.y)+a.height||x<d.clip.left||y<d.clip.top||x>=d.clip.right||y>=d.clip.bottom)continue;
        auto& p=b.pixels[std::size_t(y)*b.width+x];const auto red=std::min(((p>>11)+a.channels[0])&65535u,31u),green=std::min((((p>>5)&63)+a.channels[1])&65535u,63u),blue=std::min(((p&31)+a.channels[2])&65535u,31u);p=red<<11|green<<5|blue;
    }}return b;
}
static void save(const QString& path,const QByteArray& b){QFile f(path);require(f.open(QIODevice::NewOnly|QIODevice::WriteOnly)&&f.write(b)==b.size(),"Output write failed");}
static QByteArray pixels(const render::Image& image){QByteArray b;for(auto p:image.pixels){b.append(char(p));b.append(char(p>>8));}return b;}
static void colourRecord(QByteArray& b,unsigned mode,int x,int y,unsigned w,unsigned h,bool uniform){
    const auto at=b.size();const auto size=76+w*h*2;b.append(QByteArray(size,0));put(b,at,size);put(b,at+4,7);put(b,at+8,x);put(b,at+12,y);put(b,at+24,256);put(b,at+28,256);put(b,at+40,12+w*h*2);put(b,at+52,12);put(b,at+64,w);put(b,at+68,h);put(b,at+72,mode);
    for(unsigned i=0;i<w*h;++i){const auto word=uniform?0xd534u:(i*7919+0x1234)&65535;b[at+76+i*2]=char(word);b[at+77+i*2]=char(word>>8);}
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Expected new output prefix");const QString prefix=QString::fromLocal8Bit(argv[1]);
    QTemporaryDir directory;require(directory.isValid(),"Temporary root unavailable");auto input=store(std::filesystem::path(directory.path().toStdString()));assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
    auto valid=wire();auto frame=legacy::decodeWorldFrame(valid);require(frame.draws.size()==7&&frame.draws[0].additive&&!frame.draws[0].colours,"Primitive decoding differs");
    auto draws=legacy::worldDisplay(frame,bindings);require(draws.size()==7,"Display order lost");
    legacy::WorldPreparation worker(input);worker.plan(valid);
    auto wait=[&]{for(unsigned i=0;i<3000;++i){auto reply=worker.take();if(reply)return std::move(*reply);std::this_thread::sleep_for(std::chrono::milliseconds(1));}throw std::runtime_error("Preparation timed out");};
    const auto plan=std::get<legacy::WorldPreparationPlan>(wait());require(plan.files.empty(),"Primitive demanded asset catalogue entries");worker.prepare({},1024*1024,8);const auto ready=std::get<legacy::PreparedWorld>(wait());require(ready.resources.empty()&&ready.draws.size()==7&&ready.draws[2].additive->channels[0]==65535,"Worker primitive/order differs");worker.cancel();
    render::GlBlitter renderer;
    for(bool prepared:{false,true}){
        render::SceneLimits limits;limits.cache.indexedAtlas=true;limits.cache.residentOnly=prepared;limits.cache.preparedOnly=prepared;
        render::SceneRenderer scene(renderer,resources,background(),limits);scene.beginFrame(prepared?ready.draws:draws);while(!scene.drawNext(1)){require(!scene.uploadNeed(),"Primitive requested sprite upload");}
        require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==(prepared?1u:0u),"Normal drawing read back pixels");const auto actual=scene.read();require(actual.pixels==formula(background(),frame).pixels,"Native additive arithmetic/clipping/order differs");
        if(!prepared){save(prefix+".bin",valid);save(prefix+".before.565",pixels(background()));save(prefix+".native.565",pixels(actual));}
        auto bad=draws;bad.back().additive->width=0;rejected([&]{scene.beginFrame(bad);});require(scene.presentGpu().valid(),"Late invalid primitive discarded prior canvas");
    }
    require(renderer.stats().surfaces==0,"Primitive GPU storage leaked");
    const std::array<std::array<std::uint16_t,3>,7> increments{{{0,0,0},{1,1,1},{12,12,6},{31,63,31},{65535,65535,65535},{65520,65480,65530},{65504,65472,65504}}};
    for(std::size_t i=0;i<increments.size();++i){
        auto exhaustive=valid.first(164);put(exhaustive,16,164);put(exhaustive,24,256);put(exhaustive,28,256);put(exhaustive,32,256);put(exhaustive,36,1);
        put(exhaustive,88,0);put(exhaustive,92,0);put(exhaustive,96,0);put(exhaustive,100,0);put(exhaustive,104,256);put(exhaustive,108,256);put(exhaustive,144,256);put(exhaustive,148,256);
        for(unsigned j=0;j<3;++j)put(exhaustive,152+j*4,increments[i][j]);
        render::Image words{256,256,{}};for(unsigned p=0;p<65536;++p)words.pixels.push_back(p);
        const auto ef=legacy::decodeWorldFrame(exhaustive);render::SceneRenderer scene(renderer,resources,words);scene.draw(legacy::worldDisplay(ef,bindings));
        const auto actual=scene.read();require(actual.pixels==formula(words,ef).pixels,"Exhaustive RGB565 channel arithmetic differs");
        const auto name=prefix+"-exhaustive"+QString::number(i);save(name+".bin",exhaustive);save(name+".before.565",pixels(words));save(name+".native.565",pixels(actual));
    }
    require(renderer.stats().surfaces==0,"Exhaustive primitive surfaces leaked");
    for(unsigned mode=0;mode<4;++mode)for(bool uniform:{false,true}){
        auto colour=valid.first(80);put(colour,24,256);put(colour,28,256);put(colour,32,256);put(colour,36,16);
        for(int y=0;y<256;y+=64){for(int x=0;x<256;x+=64)colourRecord(colour,mode,x,y,64,64,uniform);}
        put(colour,16,colour.size());
        render::Image words{256,256,{}};for(unsigned i=0;i<65536;++i)words.pixels.push_back(i);
        const auto cf=legacy::decodeWorldFrame(colour);auto cd=legacy::worldDisplay(cf,bindings);
        render::SceneRenderer scene(renderer,resources,words);scene.draw(cd);const auto actual=scene.read();require(actual.pixels==formula(words,cf).pixels,"Colour source blend arithmetic differs");
        const auto name=prefix+"-colour"+QString::number(mode)+(uniform?"-uniform":"-plane");save(name+".bin",colour);save(name+".before.565",pixels(words));save(name+".native.565",pixels(actual));
        for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{84,8},{112,40},{116,1},{120,12},{132,11},{144,65},{148,0},{152,4}}){auto bad=colour;put(bad,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(bad);});}
    }
    {auto mixed=valid;put(mixed,24,256);put(mixed,28,256);put(mixed,32,256);colourRecord(mixed,2,-2,1,8,8,false);mixed.append(valid.mid(80,84));put(mixed,36,9);put(mixed,16,mixed.size());
        render::Image words{256,256,{}};for(unsigned i=0;i<65536;++i)words.pixels.push_back(i);
        auto mf=legacy::decodeWorldFrame(mixed);legacy::WorldPreparation job(input);job.plan(mixed);for(unsigned tries=0;;++tries){auto reply=job.take();if(reply){require(std::get<legacy::WorldPreparationPlan>(*reply).files.empty(),"Mixed primitive requested assets");break;}require(tries<3000,"Mixed planning timeout");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        job.prepare({},1024*1024,8);std::vector<render::SceneDraw> prepared;
        for(unsigned tries=0;;++tries){auto reply=job.take();if(reply){prepared=std::move(std::get<legacy::PreparedWorld>(*reply).draws);break;}require(tries<3000,"Mixed preparation timeout");std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        job.cancel();require(prepared.at(7).colourRectangle&&prepared.at(8).additive,"Mixed worker order differs");render::SceneRenderer scene(renderer,resources,words);scene.draw(prepared);const auto actual=scene.read();require(actual.pixels==formula(words,mf).pixels,"Mixed ordered clipping differs");const auto name=prefix+"-mixed";save(name+".bin",mixed);save(name+".before.565",pixels(words));save(name+".native.565",pixels(actual));
    }
    require(renderer.stats().surfaces==0,"Colour primitive surfaces leaked");
    for(const auto& m:std::vector<std::pair<unsigned,unsigned>>{{84,7},{112,40},{116,1},{120,0},{124,1},{128,1},{132,10},{136,1},{140,1},{144,0},{148,2049},{152,65536},{96,33},{104,33}}){auto bad=valid;put(bad,m.first,m.second);rejected([&]{legacy::decodeWorldFrame(bad);});}
    for(qsizetype n=0;n<valid.size();++n)rejected([&]{legacy::decodeWorldFrame(valid.first(n));});
    std::cout<<"Additive wire, worker without assets, ordered clipped RGB565 arithmetic, malformed refusal, no normal readback and GPU lifetime pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
