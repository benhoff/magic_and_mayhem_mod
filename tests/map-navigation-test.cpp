#include "map_navigation.hpp"
#include "route_world.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
template<class F>void refused(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected,"Invalid map binding accepted");}
int main()try{
    assets::MapAsset map;map.width=map.height=8;map.layers=4;map.cells.resize(256);
    for(unsigned i=0;i<map.cells.size();++i){auto& c=map.cells[i];c.references.fill(0xffff);if(i<64)c.definition=1;}
    map.cells[18].references={1,2,3};map.cells[18].flags10=8|0x2000;
    assets::TerrainCatalog catalog;catalog.records.resize(2);catalog.records[1][0x94]=16;catalog.records[1][0xb0]=8;
    const auto original=map.cells;
    const auto result=scene::projectMapNavigation(map,catalog,{1,1,6,6});
    check(map.cells[18].references==original[18].references && map.cells[18].flags10==original[18].flags10,"Projection changed source MAP");
    check(result.projectedObjects==1 && result.projectedReferences==3 && result.sealedCells==80,"Projection accounting wrong");
    check(result.standing.size()==16,"Ordinary supported-cell admission wrong");
    for(const auto& p:result.standing)check(p[0]>0 && p[0]<5 && p[1]>0 && p[1]<5 && p[2]==1,"Sealed crop admits edge or unsupported plane");
    const auto payload=scene::mapPayload(result.geometry);auto parsed=assets::decodeMapPayload(payload);
    check(std::holds_alternative<assets::MapAsset>(parsed),"Geometry payload is not version-6 MAP grammar");
    scene::validateMapGeometry(std::get<assets::MapAsset>(parsed),catalog,result.frozen);
    auto wrong=result.geometry;wrong.cells[20].flags8^=1;refused([&]{scene::validateMapGeometry(wrong,catalog,result.frozen);});
    wrong=result.geometry;wrong.width=5;refused([&]{scene::validateMapGeometry(wrong,catalog,result.frozen);});
    auto wrongCatalog=catalog;wrongCatalog.records[1][0xb0]^=8;refused([&]{scene::validateMapGeometry(result.geometry,wrongCatalog,result.frozen);});
    auto changed=result.frozen;changed[36]^=1;
    std::uint64_t hash=14695981039346656037ULL;for(auto b:result.frozen){hash^=b;hash*=1099511628211ULL;}
    refused([&]{scene::validateMapGeometry(result.geometry,catalog,changed,hash);});
    scene::validateMapGeometry(result.geometry,catalog,result.frozen,hash);
    refused([&]{scene::projectMapNavigation(map,catalog,{0,0,3,4});});
    refused([&]{scene::projectMapNavigation(map,catalog,{7,7,4,4});});
    auto bad=map;bad.cells[255].definition=2;refused([&]{scene::projectMapNavigation(bad,catalog,{0,0,6,6});});
    auto frozen=result.frozen;map.cells[0].definition=0;catalog.records[1][0x94]=0;
    check(frozen==result.frozen && result.geometry.cell(1,1,0).definition==1,"Projection retains borrowed source data");
    std::vector<std::byte> bytes(frozen.size());std::memcpy(bytes.data(),frozen.data(),frozen.size());
    const auto snapshot=reconstruction::decode_route_world(bytes);check(snapshot->rows[2]==12 && snapshot->layers[2]==72,"Frozen offset tables mismatch");
    const auto helpers=reconstruction::snapshot_neighbors(snapshot);
    check(helpers.node_at({1,1,1})==snapshot->cell_base+43*12,"Frozen physical token layout mismatch");
    std::cout<<"Ordinary MAP projection, sealed perimeter, owned inputs and resource mismatch refusals pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
