// Reuse the checked private PE/FS/attempt adapter; emit original grid selections
// for independent Python assembly and scene comparisons.
#define main terrainGeneratedCorpusMain
#include "terrain-generated-reference.cpp"
#undef main
int main(int argc,char** argv)try{
    if(argc!=4)throw std::runtime_error("Expected PE, one recipe fixture and seed");
    const auto seed=std::stoull(argv[3]);if(seed>0xffffffffULL)throw std::out_of_range("Seed extent");
    const auto input=read(argv[2]);require(u32(input,0)==1);unsigned at=4;
    mnm::assets::RegionRecipe recipe;recipe.id=u32(input,at);recipe.columns=u32(input,at+4);recipe.rows=u32(input,at+8);const unsigned fixed=u32(input,at+12),random=u32(input,at+16);at+=20;
    std::vector<mnm::assets::MapAsset> maps;
    for(unsigned i=0;i<fixed+random;++i){if(i<fixed)recipe.specific.push_back({u32(input,at),int(u32(input,at+4)),int(u32(input,at+8)),int(u32(input,at+12))});else recipe.random.push_back({u32(input,at),u32(input,at+16)});maps.push_back(headerMap(input,at+20));at+=96;}
    require(at==input.size());std::vector<const mnm::assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);
    const auto native=generateTerrainRegionPlan(recipe,sources,unsigned(seed));
    mapImage(read(argv[1]));installMessageImport();PrivateSeh seh;
    auto host=stateHost(makeRegionPlacementState(recipe.columns,recipe.rows,native.catalog.bank));putRequests(host,native.catalog.requests);put(host.data(),0,unsigned(seed));dialogs.clear();
    const auto status=originalAttempts(host.data());require(bool(status)==native.generation.complete);checkState(host,native.generation.state,native.generation.requests);checkLocations(host,native.generation.locations);require(u32(host,0)==native.generation.nextSeed);seh.check();
    std::cout<<"{\"complete\":"<<(status?"true":"false")<<",\"next_seed\":"<<u32(host,0)<<",\"attempts\":"<<native.generation.attempts.size()<<",\"blocks\":[";
    if(status){std::vector<unsigned> owners;for(unsigned i=0;i<maps.size();++i)for(unsigned n=0;n<maps[i].metadata[0]*maps[i].metadata[1];++n)owners.push_back(i);
        bool first=true;for(unsigned y=0;y<recipe.rows;++y)for(unsigned x=0;x<recipe.columns;++x){const unsigned cell=0x39b4+37*(x*5+y);require(host[cell]==1);const auto descriptor=u32(host,cell+5);require(descriptor<owners.size());const auto owner=owners[descriptor],side=maps[owner].width/maps[owner].metadata[0];
            if(!first)std::cout<<",";first=false;std::cout<<"{\"source\":"<<owner<<",\"source_x\":"<<u32(host,cell+13)*side<<",\"source_y\":"<<u32(host,cell+17)*side<<",\"column\":"<<x<<",\"row\":"<<y<<",\"rotation\":"<<u32(host,cell+9)<<"}";
        }
    }
    std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
