// Compare complete owned assembly to an independent Python-produced ordinary MAP.
#include "terrain_region.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
using Bytes=std::vector<unsigned char>;
static void require(bool ok){if(!ok)throw std::runtime_error("Whole region assembly differs");}
static Bytes read(const char* p){std::ifstream f(p,std::ios::binary);require(bool(f));Bytes b(std::istreambuf_iterator<char>(f),{});require(b.size()<=32*1024*1024);return b;}
static unsigned word(const Bytes& b,std::size_t at){require(at<=b.size() && b.size()-at>=4);unsigned v;std::memcpy(&v,b.data()+at,4);return v;}
static mnm::assets::MapAsset map(const Bytes& b){
 require(b.size()>=76 && word(b,0)==6);mnm::assets::MapAsset m;
 m.width=word(b,4);m.height=word(b,8);m.layers=word(b,12);
 require(m.width && m.width<=128 && m.height && m.height<=128 && m.layers && m.layers<=32);
 const auto count=std::size_t(m.width)*m.height*m.layers;require(b.size()==76+12*count);
 for(unsigned i=0;i<13;++i)m.metadata[i]=word(b,24+4*i);
 static_assert(sizeof(mnm::assets::MapCell)==12);m.cells.resize(count);std::memcpy(m.cells.data(),b.data()+76,12*count);return m;
}
int main(int argc,char** argv)try{
 require(argc==4);const auto decoded=mnm::assets::decodeTerrainCatalog(read(argv[1]));
 require(std::holds_alternative<mnm::assets::TerrainCatalog>(decoded));
 const auto bundle=read(argv[2]);mnm::assets::RegionRecipe recipe;
 recipe.columns=word(bundle,0);recipe.rows=word(bundle,4);const auto count=word(bundle,8);require(count>0 && count<=25);
 std::vector<mnm::assets::MapAsset> maps;maps.reserve(count);std::size_t at=12;
 for(unsigned i=0;i<count;++i){
  recipe.specific.push_back({word(bundle,at),int(word(bundle,at+4)),int(word(bundle,at+8)),int(word(bundle,at+12))});
  const auto size=word(bundle,at+16);at+=20;require(at<=bundle.size() && size<=bundle.size()-at);
  maps.push_back(map(Bytes(bundle.begin()+at,bundle.begin()+at+size)));at+=size;
 }
 require(at==bundle.size());std::vector<const mnm::assets::MapAsset*> sources;
 unsigned objects=0,references=0;
 for(const auto& m:maps){sources.push_back(&m);for(const auto& cell:m.cells){objects+=bool(cell.flags10&8);for(auto ref:cell.references)references+=ref!=65535;}}
 const auto plan=mnm::reconstruction::planFixedTerrainRegion(recipe,sources);
 const auto actual=mnm::reconstruction::assembleFixedTerrainRegion(plan,sources,std::get<mnm::assets::TerrainCatalog>(decoded));
 const auto expected=map(read(argv[3]));require(actual.map.width==expected.width && actual.map.height==expected.height && actual.map.layers==expected.layers && actual.map.cells.size()==expected.cells.size());
 for(std::size_t i=0;i<expected.cells.size();++i){const auto& a=actual.map.cells[i];const auto& e=expected.cells[i];require(a.definition==e.definition && a.references==e.references && a.flags8==e.flags8 && a.flags10==e.flags10);}
 require(actual.projectedObjects==objects && actual.projectedReferences==references);
 std::cout<<"{\"all_match\":true,\"cells\":"<<expected.cells.size()<<",\"blocks\":"<<plan.blocks.size()<<",\"objects\":"<<objects<<",\"references\":"<<references<<"}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
