#include "../compat/legacy/canvas_world.hpp"
#include "../compat/legacy/world_raster_batch.hpp"
#include "../protocols/include/mnm/world_frame_v1.h"
#include "resource-fixtures.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <algorithm>
using namespace mnm;
using namespace resource_test;
static QByteArray load(const QString& path){QFile f(path);require(f.open(QIODevice::ReadOnly),"Producer fixture input");return f.readAll();}
static unsigned get(const QByteArray& b,unsigned at){require(at+4<=unsigned(b.size()),"Fixture WORD");const auto* p=reinterpret_cast<const unsigned char*>(b.constData()+at);return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
static void same(const legacy::WorldDraw& a,const legacy::WorldDraw& b){
    require(a.x==b.x&&a.y==b.y&&a.backend==b.backend&&a.frame.indexed==b.frame.indexed&&a.frame.encoded==b.frame.encoded,"Producer changed frame/anchor/backend");
    require(a.clip.left==b.clip.left&&a.clip.top==b.clip.top&&a.clip.right==b.clip.right&&a.clip.bottom==b.clip.bottom,"Producer changed clip");
    require(a.colours==b.colours&&a.composite.mode==b.composite.mode&&bool(a.rowEffect)==bool(b.rowEffect),"Producer changed palette/composition");
    if(a.rowEffect)require(a.rowEffect->kind==b.rowEffect->kind&&a.rowEffect->wholeRunClip==b.rowEffect->wholeRunClip&&a.rowEffect->offsets==b.rowEffect->offsets,"Producer lost owned row inputs");
}
static Bytes sprite(const std::vector<Bytes>& frames,bool indexed){
    const unsigned base=24+(indexed?768:0)+frames.size()*4;Bytes result(base,0);
    word(result,0,0x00525053);word(result,8,4);word(result,12,frames.size());word(result,16,indexed?1:0);
    for(unsigned i=0;i<frames.size();++i){word(result,24+(indexed?768:0)+i*4,result.size()-base);result.insert(result.end(),frames[i].begin(),frames[i].end());}
    word(result,4,result.size());return result;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==2,"Expected producer fixture manifest");
    const auto items=QJsonDocument::fromJson(load(QString::fromLocal8Bit(argv[1]))).array();require(!items.empty(),"Empty producer corpus");
    render::GlBlitter renderer;unsigned cases=0,draws=0;std::uint64_t pixels=0;
    for(const auto& item:items){const auto o=item.toObject();const auto wire=load(o["input"].toString());auto frame=legacy::decodeWorldFrame(wire);
        unsigned at=80;bool selected=false;for(unsigned i=0;i<frame.draws.size();++i){const auto op=get(wire,at+4);selected|=op>=8;at+=get(wire,at);}
        if(!selected)continue;
        QTemporaryDir directory;require(directory.isValid(),"Producer asset root");const auto root=std::filesystem::path(directory.path().toStdString());
        const auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
        std::vector<Bytes> indexed,direct;for(const auto& d:frame.draws)if(!d.frame.encoded.isEmpty()){
            auto& target=d.frame.indexed?indexed:direct;const Bytes encoded(d.frame.encoded.begin(),d.frame.encoded.end());
            if(std::find(target.begin(),target.end(),encoded)==target.end())target.push_back(encoded);}
        for(bool isIndexed:{false,true}){const auto& source=isIndexed?indexed:direct;if(source.empty())continue;
            const auto file=sprite(source,isIndexed);const auto name=isIndexed?"indexed.spr":"direct.spr";write(root,name,file);
            bindings.add({assets::ResourceKind::ui,isIndexed?"producer/indexed":"producer/direct"},name,QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(file.data()),file.size()),QCryptographicHash::Sha256).toHex());}
        at=80;
        for(unsigned i=0;i<frame.draws.size();++i){const auto op=get(wire,at+4),encoded=get(wire,at+32),payload=get(wire,at+40);require(op>=8&&op<=12,"Mixed non-effect fixture needs a separate adapter");
            legacy::CanvasProducer c{};auto& r=c.fields;r[0]=96+encoded+payload;r[1]=i+1;r[2]=9;r[3]=1;r[5]=frame.width;r[6]=frame.height;r[7]=frame.stride;
            r[8]=get(wire,at+8);r[9]=get(wire,at+12);for(unsigned k=0;k<4;++k)r[10+k]=get(wire,at+16+k*4);
            r[14]=op;r[15]=get(wire,at+52);r[16]=get(wire,at+44);r[17]=get(wire,at+36);r[18]=encoded+payload;r[19]=encoded;r[20]=payload;
            c.payload.assign(wire.begin()+at+64,wire.begin()+at+64+encoded+payload);
            // Exercise the versioned owned definition/reference decoder too.
            std::vector<std::uint8_t> stream(64);std::copy_n("MNMPRO03",8,stream.begin());
            auto put=[&](unsigned where,unsigned value){for(unsigned k=0;k<4;++k)stream.at(where+k)=std::uint8_t(value>>(k*8));};
            put(8,3);put(12,64);put(16,0x40209ca7);put(20,131072);put(24,32);
            auto append=[&](std::array<std::uint32_t,24> fields,const std::vector<std::uint8_t>& data){
                fields[0]=96+data.size();fields[18]=data.size();const auto begin=stream.size();stream.resize(begin+96);
                for(unsigned k=0;k<24;++k)put(begin+k*4,fields[k]);
                stream.insert(stream.end(),data.begin(),data.end());};
            std::array<std::uint32_t,24> definition{};definition[1]=1;definition[2]=25;definition[14]=9;definition[17]=r[17];definition[19]=r[19];definition[20]=r[20];
            append(definition,c.payload);auto reference=r;reference[1]=2;reference[4]=1;append(reference,{});
            const auto restored=legacy::decodeCanvasProducers(stream,false);require(restored.operations.size()==2,"Effect source definition/reference extent");
            same(legacy::producerWorldDraw(restored.operations.back()),frame.draws[i]);
            auto decoded=legacy::producerWorldDraw(c);same(decoded,frame.draws[i]);require(legacy::worldRasterAX(c)==0,"Producer effect AX");
            c.ownedPayload=std::make_shared<const std::vector<std::uint8_t>>(std::move(c.payload));decoded=legacy::producerWorldDraw(c);same(decoded,frame.draws[i]);
            auto bad=c;bad.fields[15]=12;rejected([&]{legacy::producerWorldDraw(bad);});rejected([&]{legacy::worldRasterAX(bad);});
            bad=c;bad.fields[14]=7;rejected([&]{legacy::producerWorldDraw(bad);});
            bad=c;bad.fields[20]++;rejected([&]{legacy::producerWorldDraw(bad);});
            frame.draws[i]=std::move(decoded);at+=get(wire,at);++draws;
        }
        const auto before=load(o["before"].toString()),expected=load(o["expected"].toString());require(before.size()==int(frame.width*frame.height*2),"Producer prior extent");
        render::Image canvas{int(frame.width),int(frame.height),{}};for(int p=0;p<before.size();p+=2)canvas.pixels.push_back(std::uint8_t(before[p])|unsigned(std::uint8_t(before[p+1]))<<8);
        render::SceneRenderer scene(renderer,resources,canvas);const auto reads=renderer.stats().nativeReadbacks;scene.draw(legacy::worldDisplay(frame,bindings));
        require(scene.presentGpu().valid()&&renderer.stats().nativeReadbacks==reads,"Producer normal path readback");const auto actual=scene.read();QByteArray output;
        for(auto p:actual.pixels){output.append(char(p));output.append(char(p>>8));}require(output==expected,"Producer GPU pixels differ from original");
        pixels+=actual.pixels.size();++cases;
    }
    require(cases&&draws,"No selected effect producer fixtures");std::cout<<"{\"success\":true,\"fixtures\":"<<cases<<",\"draws\":"<<draws<<",\"pixels\":"<<pixels<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
