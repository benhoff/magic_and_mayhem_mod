#include "effect_lighting.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected asset root and field output");
 auto store=AssetStore::create(argv[1]);if(auto* e=std::get_if<Error>(&store))throw std::runtime_error(e->detail);
 auto file=std::get<AssetStore>(store).open("cFg\\eNcRyPtEd\\EfFeCtS.cFg");if(auto* e=std::get_if<Error>(&file))throw std::runtime_error(e->detail);
 auto result=loadEffectLightingFields(*std::get<std::unique_ptr<AssetFile>>(file));if(auto* e=std::get_if<PersistenceError>(&result))throw std::runtime_error(e->detail);
 std::ofstream output(argv[2],std::ios::binary);
 for(const auto& row:std::get<EffectLightingFields>(result).values)for(const auto& value:row){unsigned n=value.size();output.write(reinterpret_cast<char*>(&n),4);output.write(value.data(),n);}
 if(!output)throw std::runtime_error("Field output failure");
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
