#include "map_navigation.hpp"
#include "terrain_map.hpp"
#include "route_world.hpp"
#include "route_cell_support.hpp"
#include "creature_profile.hpp"
#include <cstring>
#include <stdexcept>
namespace mnm::scene {
namespace {
using Bytes=assets::Bytes;
void word(Bytes& b,std::uint32_t v){for(unsigned i=0;i<4;++i)b.push_back(std::uint8_t(v>>(8*i)));}
void half(Bytes& b,std::uint16_t v){b.push_back(v&255);b.push_back(v>>8);}
void put(Bytes& b,unsigned at,std::uint32_t v){for(unsigned i=0;i<4;++i)b.at(at+i)=std::uint8_t(v>>(8*i));}
Bytes cellBytes(const assets::MapAsset& m){
    Bytes b;b.reserve(m.cells.size()*12);
    for(const auto& c:m.cells){half(b,c.definition);for(auto ref:c.references)half(b,ref);half(b,c.flags8);half(b,c.flags10);}
    return b;
}
void bounds(const assets::MapAsset& m){
    if(m.width<4 || m.height<4 || m.width>16 || m.height>16 || m.layers<2 || m.layers>32 ||
       std::uint64_t(m.width)*m.height*m.layers>4096 || m.cells.size()!=std::size_t(m.width)*m.height*m.layers)
        throw std::invalid_argument("Map navigation requires 4..16 XY, 2..32 layers and at most 4096 cells");
}
std::shared_ptr<reconstruction::RouteWorldSnapshot> decode(const Bytes& bytes){
    std::vector<std::byte> b(bytes.size());std::memcpy(b.data(),bytes.data(),bytes.size());
    return reconstruction::decode_route_world(b);
}
}
assets::Bytes mapPayload(const assets::MapAsset& m){
    bounds(m);Bytes b;word(b,6);word(b,m.width);word(b,m.height);word(b,m.layers);
    word(b,m.width*m.height);word(b,std::uint32_t(m.cells.size()));
    // Source-world header metadata is intentionally not rebound to this crop.
    for(unsigned i=0;i<13;++i)word(b,0);
    const auto cells=cellBytes(m);b.insert(b.end(),cells.begin(),cells.end());return b;
}
void validateMapGeometry(const assets::MapAsset& m,const assets::TerrainCatalog& t,const Bytes& bytes,std::optional<std::uint64_t> fingerprint){
    bounds(m);
    if(fingerprint){
        std::uint64_t hash=14695981039346656037ULL;
        for(auto byte:bytes){hash^=byte;hash*=1099511628211ULL;}
        if(hash!=*fingerprint)throw std::invalid_argument("Frozen MAP changed after resource admission");
    }
    const auto world=decode(bytes);
    if(world->dimensions.x!=int(m.width) || world->dimensions.y!=int(m.height) || world->dimensions.z!=int(m.layers) ||
       world->plane_stride!=int(m.width*m.height) || world->cells.size()!=m.cells.size() || world->terrain.size()!=t.records.size())
        throw std::invalid_argument("Visual MAP dimensions/catalog differ from frozen navigation");
    for(unsigned y=0;y<m.height;++y)if(world->rows[y]!=int(y*m.width))throw std::invalid_argument("Frozen MAP row layout mismatch");
    for(unsigned z=0;z<m.layers;++z)if(world->layers[z]!=int(z*m.width*m.height))throw std::invalid_argument("Frozen MAP layer layout mismatch");
    const auto cells=cellBytes(m);
    if(std::memcmp(cells.data(),world->cells.data(),cells.size()) ||
       std::memcmp(t.records.data(),world->terrain.data(),t.records.size()*356))
        throw std::invalid_argument("Visual MAP/TTD bytes differ from frozen navigation");
}
MapNavigation projectMapNavigation(const assets::MapAsset& source,const assets::TerrainCatalog& t,MapCrop crop){
    if(crop.width<4 || crop.height<4 || crop.width>16 || crop.height>16 ||
       crop.x>=source.width || crop.y>=source.height || crop.width>source.width-crop.x || crop.height>source.height-crop.y ||
       source.layers<2 || std::uint64_t(crop.width)*crop.height*source.layers>4096)
        throw std::invalid_argument("Bounded MAP crop outside source or cell budget");
    // Derive before cropping: interior geometry retains source neighbor context.
    const auto prepared=reconstruction::prepareTerrainGeometry(source,t);
    MapNavigation result;result.projectedObjects=prepared.projectedObjects;result.projectedReferences=prepared.projectedReferences;
    auto& m=result.geometry;m.width=crop.width;m.height=crop.height;m.layers=source.layers;
    for(unsigned z=0;z<m.layers;++z)for(unsigned y=0;y<m.height;++y)for(unsigned x=0;x<m.width;++x){
        auto c=prepared.map.cell(crop.x+x,crop.y+y,z);
        if(!x || !y || x+1==m.width || y+1==m.height){c.flags10|=0x4000;++result.sealedCells;}
        m.cells.push_back(c);
    }
    bounds(m);
    std::array<Bytes,7> blocks;
    for(unsigned y=0;y<m.height;++y)word(blocks[0],y*m.width);
    for(unsigned z=0;z<m.layers;++z)word(blocks[1],z*m.width*m.height);
    blocks[2]=cellBytes(m);
    for(const auto& record:t.records)blocks[3].insert(blocks[3].end(),record.begin(),record.end());
    blocks[4].resize(0xd07);put(blocks[4],8,1);put(blocks[4],12,1);put(blocks[4],16,1);
    blocks[5].resize(0x198);put(blocks[5],8,1);put(blocks[5],12,1);put(blocks[5],16,500);
    for(unsigned i=0;i<48;++i)put(blocks[5],0xd8+4*i,(i/12)%2?40:60);
    blocks[6].resize(0x5c9);put(blocks[6],0x589,720);
    float slope=0.969F;std::uint32_t slopeBits;std::memcpy(&slopeBits,&slope,4);
    Bytes b{'M','N','M','W','L','D','0','1'};
    for(auto v:std::array<std::uint32_t,14>{0x1200000,m.width,m.height,m.layers,m.width*m.height,1,300,0,1,1,1,720,slopeBits,0x1000000})word(b,v);
    for(const auto& block:blocks)word(b,std::uint32_t(block.size()));
    for(const auto& block:blocks)b.insert(b.end(),block.begin(),block.end());
    result.frozen=std::move(b);
    const auto world=decode(result.frozen);
    const reconstruction::CellValidityMapView view{{world->dimensions,world->cells.data(),world->cells.size(),world->rows.data(),world->rows.size(),world->layers.data(),world->layers.size(),world->plane_stride,world->dimensions.z},world->terrain.data(),world->terrain.size()};
    const reconstruction::CreatureMovementParameters p{1,1,0,0,0,0,0};
    for(unsigned z=1;z<m.layers;++z)for(unsigned y=1;y+1<m.height;++y)for(unsigned x=1;x+1<m.width;++x){
        const reconstruction::Coordinates at{int(x),int(y),int(z)};
        const auto height=world->terrain[m.cell(x,y,z).definition].classification_94;
        if(height>=-16 && height<=16 && reconstruction::test_cell_validity(at,p,view) && reconstruction::test_cell_support(at,p,view))
            result.standing.push_back({int(x),int(y),int(z)});
    }
    validateMapGeometry(m,t,result.frozen);return result;
}
MapNavigation projectCreatureMapNavigation(const assets::MapAsset& source,const assets::TerrainCatalog& t,MapCrop crop,const assets::CreatureMovementConfig& c,const assets::Animation& ani){
    if(c.type!=10 || c.width!=1 || c.canFly || c.swimming!=1 || c.height<1 || c.height>5 || c.acceleration<0 || c.acceleration>1000000)
        throw std::invalid_argument("Configured navigation currently admits ordinary ground Redcap only");
    const auto samples=reconstruction::groundMovementSamples(ani);
    for(auto sample:samples)if(sample>192)throw std::invalid_argument("Configured movement sample exceeds native driver bound");
    const auto maximum=reconstruction::groundMovementMaximum(samples);
    auto result=projectMapNavigation(source,t,crop);auto& b=result.frozen;
    unsigned cursor=92;
    for(unsigned i=0;i<4;++i){std::uint32_t n=0;std::memcpy(&n,b.data()+64+i*4,4);cursor+=n;}
    const auto object=cursor,type=object+0xd07,generator=type+0x198;
    put(b,object+0xa8,c.type);put(b,type+8,c.width);put(b,type+12,c.height);put(b,type+16,c.acceleration);put(b,type+0x44,c.swimming);
    for(unsigned i=0;i<48;++i)put(b,type+0xd8+4*i,samples[i]);
    put(b,generator+0x589,maximum);
    // All projected cell/TTD bytes and default global policies remain identical.
    const auto world=decode(b);result.standing.clear();
    const reconstruction::CellValidityMapView view{{world->dimensions,world->cells.data(),world->cells.size(),world->rows.data(),world->rows.size(),world->layers.data(),world->layers.size(),world->plane_stride,world->dimensions.z},world->terrain.data(),world->terrain.size()};
    const reconstruction::CreatureMovementParameters p{c.height,c.width,0,c.swimming,0,int(c.type),0};
    const auto& m=result.geometry;
    for(unsigned z=1;z<m.layers;++z)for(unsigned y=1;y+1<m.height;++y)for(unsigned x=1;x+1<m.width;++x){
        const reconstruction::Coordinates at{int(x),int(y),int(z)};
        const auto height=world->terrain[m.cell(x,y,z).definition].classification_94;
        if(height>=-16 && height<=16 && reconstruction::test_cell_validity(at,p,view) && reconstruction::test_cell_support(at,p,view))result.standing.push_back({int(x),int(y),int(z)});
    }
    validateMapGeometry(m,t,b);return result;
}

}
