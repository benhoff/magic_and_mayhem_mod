#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include "../compat/legacy/world_continuous.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <cstdio>
#include <stdexcept>
#include <algorithm>
using namespace mnm;
namespace {
void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
std::vector<std::uint8_t> area(const QByteArray& b,qsizetype at,qsizetype n){require(at>=0 && n>=0 && n<=b.size()-at,"Captured area bounds");return {reinterpret_cast<const std::uint8_t*>(b.constData()+at),reinterpret_cast<const std::uint8_t*>(b.constData()+at+n)};}
void put(std::vector<std::uint8_t>& b,unsigned at,unsigned w){for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(w>>(i*8));}
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);require(argc==4,"Expected captured channel,asset root,report");QFile input(argv[1]);
    require(input.open(QIODevice::ReadOnly) && input.size()==MNM_CONT_BYTES,"Captured resident channel extent");const auto bytes=input.readAll();
    require(bytes.first(8)==MNM_CONT_RESIDENT_MAGIC,"Captured resident magic");auto word=[&](qsizetype at){require(at>=0 && at+4<=bytes.size(),"Captured word bounds");return qFromLittleEndian<quint32>(bytes.constData()+at);};
    auto opened=assets::AssetStore::create(argv[2]);if(auto* error=std::get_if<assets::Error>(&opened))throw std::runtime_error(error->detail);
    const auto store=std::get<assets::AssetStore>(std::move(opened));render::GlBlitter renderer;
    unsigned words=0,rasters=0,wordRasters=0,primitives=0,refusals=0;
    {
        legacy::WorldRollingSession cpu(renderer,store,true),gpu(renderer,store,false,true);
        legacy::CanvasProducerStream last;
        for(unsigned generation=1;generation<=2;++generation){const auto slot=qsizetype(MNM_CONT_HEADER)+(generation-1)*MNM_CONT_SLOT_BYTES;
            require(word(slot+8)==generation && !word(slot+12),"Captured generation");const auto sourceAt=slot+MNM_CONT_SLOT_HEADER;
            const auto source=area(bytes,sourceAt,word(slot+16)),request=area(bytes,sourceAt+MNM_CONT_SOURCE_BYTES,word(slot+20));
            auto stream=legacy::decodeCanvasProducers(source,false);auto handoff=legacy::prepareRollingHandoff(stream,0,request,1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
            const auto expected=cpu.consumeOwned(generation,handoff.stream);const auto before=renderer.stats();gpu.consumeResident(generation,handoff.stream);
            require(renderer.stats().nativeReadbacks==before.nativeReadbacks && renderer.stats().rgbaReadbacks==before.rgbaReadbacks && !gpu.worldReadbacks(),"Captured block delivery read back");
            const auto actual=gpu.diagnosticReadResident(renderer);require(actual.width==expected.width && actual.height==expected.height && actual.pixels==expected.pixels,"Captured resident/CPU World words differ");words+=actual.pixels.size();
            for(const auto& operation:handoff.stream.operations)if(operation.fields[2]==9){++rasters;if(operation.fields[15]==8)++wordRasters;}
            last=handoff.stream;
        }
        // Mix destination-dependent primitives with sprites. The independent CPU
        // reference catches ordering errors; the resident path must retain no
        // CPU destination seed or delivery readback.
        legacy::CanvasProducerStream mixed;mixed.version=3;mixed.queues=1;
        const auto entry=*std::find_if(last.operations.begin(),last.operations.end(),[](const auto& c){return c.fields[2]==11;});
        const auto sprite=*std::find_if(last.operations.begin(),last.operations.end(),[](const auto& c){return c.fields[2]==9 && c.fields[15]==8;});
        mixed.operations.push_back(entry);
        auto add=[&](unsigned red){auto c=entry;c.fields[2]=14;c.fields[8]=100;c.fields[9]=100;c.fields[10]=0;c.fields[11]=0;c.fields[12]=800;c.fields[13]=600;c.fields[14]=0;c.fields[18]=20;c.fields[0]=116;c.payload.assign(20,0);put(c.payload,0,16);put(c.payload,4,16);put(c.payload,8,red);put(c.payload,12,3);put(c.payload,16,7);mixed.operations.push_back(std::move(c));++primitives;};
        add(4);auto s=sprite;s.fields[8]=100;s.fields[9]=100;mixed.operations.push_back(s);
        for(unsigned mode=0;mode<4;++mode){auto c=entry;c.fields[2]=26;c.fields[8]=96;c.fields[9]=96;c.fields[10]=98;c.fields[11]=99;c.fields[12]=800;c.fields[13]=600;c.fields[14]=0;c.fields[15]=mode;c.fields[19]=16;c.fields[20]=16;c.fields[18]=512;c.fields[0]=608;c.payload.resize(512);for(unsigned i=0;i<256;++i){unsigned v=(i*97+mode*3001)&65535;c.payload[i*2]=v;c.payload[i*2+1]=v>>8;}mixed.operations.push_back(std::move(c));++primitives;}
        add(31);mixed.operations.push_back(s);
        auto checkpoint=*std::find_if(last.operations.begin(),last.operations.end(),[](const auto& c){return c.fields[2]==10 && c.fields[15]==4;});mixed.operations.push_back(checkpoint);mixed.operations.push_back(last.operations.back());
        for(unsigned i=0;i<mixed.operations.size();++i)mixed.operations[i].fields[1]=i+1;
        const auto encoded=legacy::encodeRollingQueue(mixed.operations);mixed=legacy::decodeRollingQueue(encoded);
        const auto expected=cpu.consumeOwned(3,mixed);const auto before=renderer.stats();gpu.consumeResident(3,mixed);
        require(renderer.stats().nativeReadbacks==before.nativeReadbacks && renderer.stats().rgbaReadbacks==before.rgbaReadbacks && !gpu.worldReadbacks(),"Mixed primitive delivery readback");
        const auto actual=gpu.diagnosticReadResident(renderer);require(actual.pixels==expected.pixels,"Mixed ordered primitives differ");words+=actual.pixels.size();
        for(unsigned which=0;which<4;++which){auto bad=mixed;auto& c=*std::find_if(bad.operations.begin(),bad.operations.end(),[](const auto& c){return c.fields[2]==26;});if(which==0)c.fields[15]=4;if(which==1)c.fields[19]=65;if(which==2)c.payload.pop_back();if(which==3)c.fields[14]=1;try{legacy::producerWorldDraw(c);}catch(const std::invalid_argument&){++refusals;}}
        require(refusals==4,"Malformed colour primitive admitted");
    }
    require(wordRasters && !renderer.stats().surfaces,"Missing WORD branch or leaked surfaces");
    QJsonObject result{{"success",true},{"queues",3},{"words_compared",int(words)},{"rasters",int(rasters)},{"word_rasters",int(wordRasters)},{"mixed_primitives",int(primitives)},{"colour_refusals",int(refusals)},
        {"delivery_readbacks",0},{"independent_original_equivalence",false},{"remaining_surfaces",0}};
    QFile output(argv[3]);require(output.open(QIODevice::WriteOnly|QIODevice::NewOnly) && output.write(QJsonDocument(result).toJson())>0,"New comparison report");
    std::printf("%u World words,%u WORD rasters,zero delivery readbacks\n",words,wordRasters);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
