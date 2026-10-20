#include "world_preparation.hpp"
#include "resource-fixtures.hpp"
#include "../protocols/include/mnm/world_frame_v1.h"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <QFile>
#include <chrono>
#include <thread>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static QByteArray bytes(const Bytes& b){return {reinterpret_cast<const char*>(b.data()),qsizetype(b.size())};}
static void put(QByteArray& b,qsizetype at,std::uint32_t v){for(unsigned i=0;i<4;++i)b[at+i]=char(v>>(i*8));}
struct Fixture {
    int width,height,originX,originY;
    Bytes mask,indices,encoded;
};
static Fixture fixture(int w,int h,bool holes){
    Fixture f{w,h,holes?3:0,holes?-2:0,{},{},Bytes(40+h*8,0)};
    std::vector<Bytes> pixelRows;
    word(f.encoded,4,w);word(f.encoded,8,h);word(f.encoded,12,f.originX);word(f.encoded,16,f.originY);
    for(int y=0;y<h;++y){
        Bytes runs,pixels;int x=0;
        while(x<w){
            unsigned skip=0,run=0;
            while(x<w&&holes&&(x+y*2)%7>=4){++skip;f.mask.push_back(0);f.indices.push_back(0);++x;}
            runs.push_back(skip);
            if(x==w)break;
            while(x<w&&run<255&&(!holes||(x+y*2)%7<4)){
                const auto index=std::uint8_t(x*23+y*29);pixels.push_back(index);
                f.mask.push_back(1);f.indices.push_back(index);++run;++x;
            }
            runs.push_back(run);
        }
        word(f.encoded,40+y*8,f.encoded.size());f.encoded.insert(f.encoded.end(),runs.begin(),runs.end());
        pixelRows.push_back(std::move(pixels));
    }
    for(int y=0;y<h;++y){word(f.encoded,44+y*8,f.encoded.size());f.encoded.insert(f.encoded.end(),pixelRows[y].begin(),pixelRows[y].end());}
    word(f.encoded,0,f.encoded.size());return f;
}
static Bytes spriteFile(const std::vector<Fixture>& frames){
    const auto base=24+768+frames.size()*4;Bytes b(base,0);
    word(b,0,0x00525053);word(b,8,4);word(b,12,frames.size());word(b,16,1);
    for(std::size_t i=0;i<frames.size();++i){word(b,792+i*4,b.size()-base);b.insert(b.end(),frames[i].encoded.begin(),frames[i].encoded.end());}
    word(b,4,b.size());return b;
}
static QByteArray envelope(int w,int h){
    QByteArray b(80,0);b.replace(qsizetype(0),qsizetype(8),MNM_WORLD_MAGIC,qsizetype(8));
    put(b,8,1);put(b,12,80);put(b,20,1);put(b,24,w);put(b,28,h);put(b,32,w);put(b,44,MNM_WORLD_BUILD);return b;
}
static void record(QByteArray& b,const Fixture& f,int x,int y,render::Rect clip,const render::SpriteColourTable& palette){
    const auto at=b.size();b.append(QByteArray(64,0));put(b,at,64+f.encoded.size()+512);put(b,at+4,MNM_WORLD_ADDITIVE_SPRITE);
    put(b,at+8,x);put(b,at+12,y);put(b,at+16,clip.left);put(b,at+20,clip.top);put(b,at+24,clip.right);put(b,at+28,clip.bottom);
    put(b,at+32,f.encoded.size());put(b,at+36,1);put(b,at+40,512);put(b,at+52,13);b.append(bytes(f.encoded));
    for(auto p:palette){b.append(char(p));b.append(char(p>>8));}
}
static void seal(QByteArray& b,unsigned count){put(b,16,b.size());put(b,36,count);}
static render::Image background(int w,int h){render::Image b{w,h,{}};for(unsigned i=0;i<unsigned(w*h);++i)b.pixels.push_back(i&65535);return b;}
static void apply(render::Image& b,const Fixture& f,int x,int y,render::Rect clip,const render::SpriteColourTable& palette){
    x-=f.originX;y-=f.originY;
    for(int sy=0;sy<f.height;++sy)for(int sx=0;sx<f.width;++sx){
        const int dx=x+sx,dy=y+sy;const auto i=std::size_t(sy)*f.width+sx;
        if(!f.mask[i]||dx<clip.left||dy<clip.top||dx>=clip.right||dy>=clip.bottom)continue;
        auto& d=b.pixels[std::size_t(dy)*b.width+dx];const auto s=palette[f.indices[i]];
        d=(std::min((d>>11)+(s>>11),31u)<<11)|(std::min(((d>>5)&63)+((s>>5)&63),63u)<<5)|std::min((d&31)+(s&31),31u);
    }
}
static QByteArray pixels(const render::Image& b){QByteArray out;for(auto p:b.pixels){out.append(char(p));out.append(char(p>>8));}return out;}
static void save(const QString& path,const QByteArray& b){QFile f(path);require(f.open(QIODevice::NewOnly|QIODevice::WriteOnly)&&f.write(b)==b.size(),"Cannot save additive fixture");}
static legacy::WorldPreparationReply await(legacy::WorldPreparation& worker){
    for(unsigned tries=0;tries<3000;++tries){auto r=worker.take();if(r)return std::move(*r);std::this_thread::sleep_for(std::chrono::milliseconds(1));}
    throw std::runtime_error("Additive preparation timeout");
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Expected fresh output prefix");const QString prefix=QString::fromLocal8Bit(argv[1]);
    const auto dense=fixture(256,256,false),holes=fixture(17,9,true);const auto sprBytes=spriteFile({dense,holes});
    QTemporaryDir directory;require(directory.isValid(),"Fixture root unavailable");const auto root=std::filesystem::path(directory.path().toStdString());write(root,"additive.spr",sprBytes);
    auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
    bindings.add({assets::ResourceKind::ui,"additive"},"additive.spr",QCryptographicHash::hash(bytes(sprBytes),QCryptographicHash::Sha256).toHex());
    render::GlBlitter renderer;unsigned cases=0;
    auto check=[&](QByteArray wire,const render::Image& before,const render::Image& expected,bool worker){
        const auto frame=legacy::decodeWorldFrame(wire);auto draws=legacy::worldDisplay(frame,bindings);
        if(worker){
            legacy::WorldPreparation job(input);job.plan(wire);const auto plan=std::get<legacy::WorldPreparationPlan>(await(job));
            require(plan.files.size()==1,"Additive sprite lost owned asset plan");
            resources.bind(plan.files[0].id,spr(plan.files[0].path));
            job.prepare({resources.request(plan.files[0].id)},16*1024*1024,8);
            auto ready=std::get<legacy::PreparedWorld>(await(job));require(ready.draws.size()==draws.size()&&ready.draws[0].composite.mode==render::CompositeMode::additive,"Worker lost additive mode");job.cancel();
        }
        render::SceneLimits limits;limits.cache.indexedAtlas=worker;
        render::SceneRenderer scene(renderer,resources,before,limits);const auto reads=renderer.stats().nativeReadbacks;
        scene.draw(draws);require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Additive drawing reads back pixels");
        const auto actual=scene.read();require(actual.pixels==expected.pixels,"Additive sprite arithmetic, coverage, clipping or order differs");
        const auto name=prefix+"-"+QString::number(cases++);save(name+".bin",wire);save(name+".before.565",pixels(before));save(name+".native.565",pixels(actual));
        auto invalid=draws;invalid.back().composite.mode=render::CompositeMode(6);rejected([&]{scene.beginFrame(invalid);});require(scene.read().pixels==actual.pixels,"Rejected additive frame changed canvas");
    };
    for(unsigned s=0;s<64;++s){
        render::SpriteColourTable palette;palette.fill(((s&31)<<11)|(s<<5)|(s&31));
        auto wire=envelope(256,256);record(wire,dense,0,0,{0,0,256,256},palette);seal(wire,1);
        constexpr int shades[]{-127,-1,0,1,127};put(wire,124,shades[s%5]);put(wire,136,s%3);
        const auto before=background(256,256);auto expected=before;apply(expected,dense,0,0,{0,0,256,256},palette);check(wire,before,expected,s==0);
        if(s==0){
            for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{84,9},{112,39},{116,0},{116,2},{120,0},{132,0},{132,12},{132,14},{140,1},{172,1}}){auto bad=wire;put(bad,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(bad);});}
            for(qsizetype n:{qsizetype(0),qsizetype(79),qsizetype(143),wire.size()-1})rejected([&]{legacy::decodeWorldFrame(wire.first(n));});
        }
    }
    for(bool narrowed:{false,true}){
        auto before=background(37,19),expected=before;auto wire=envelope(37,19);unsigned count=0;
        render::SpriteColourTable palette;for(unsigned i=0;i<256;++i)palette[i]=(i*7919+0x1234)&65535;palette[0]=0;
        const render::Rect clip=narrowed?render::Rect{4,3,29,16}:render::Rect{0,0,37,19};
        for(int y:{-12,-1,0,2,7,12,19})for(int x:{-20,-1,0,2,7,18,21,37}){
            record(wire,holes,x,y,clip,palette);apply(expected,holes,x,y,clip,palette);++count;
        }
        seal(wire,count);check(wire,before,expected,true);
    }
    require(renderer.stats().surfaces==0,"Additive sprite GPU lifetime leak");
    std::cout<<cases<<" additive fixtures: all channel pairs, all destination words, indexed zero/holes, signed anchors, ordered clipping, wire/worker and GPU lifetime pass\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
