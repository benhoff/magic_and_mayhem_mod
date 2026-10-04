#include "region_recipe.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
static Bytes bytes(const std::string& s){return Bytes(s.begin(),s.end());}
static void require(bool b){if(!b)throw std::runtime_error("Region recipe contract");}
int main(int argc,char** argv)try{
 if(argc==2){auto s=AssetStore::create(argv[1]);if(auto* e=std::get_if<Error>(&s))throw std::runtime_error(e->detail);unsigned total=0;
  for(const auto* realm:{"Celtic","Greek","Medieval"}){auto file=std::get<AssetStore>(s).open(std::string("Realms/")+realm+"/"+realm+".cfg");if(auto* e=std::get_if<Error>(&file))throw std::runtime_error(e->detail);auto r=loadRegionRecipes(*std::get<std::unique_ptr<AssetFile>>(file));if(auto* e=std::get_if<PersistenceError>(&r))throw std::runtime_error(e->detail);total+=std::get<std::vector<RegionRecipe>>(r).size();}
  require(total==40);std::cout<<"40 installed region recipes loaded\n";return 0;
 }
 const std::string source="; example\n[Region6] ; inline\nName = Marsh\nPath=Realms\\Celtic\\Swamp\nSectionPreFix=CSsec\nMapSize=(4,4)\nSpecific=(01,0,2,2)(2,-1,-1,-1)\nRandom=(10_2,10_3)\nUnrelated=1\nUnrelated=2\n";
 auto r=decodeRegionRecipes(bytes(source));require(std::holds_alternative<std::vector<RegionRecipe>>(r));auto recipes=std::get<std::vector<RegionRecipe>>(r);require(recipes.size()==1 && recipes[0].id==6 && recipes[0].specific[1].rotation==-1 && recipes[0].random.size()==2 && recipes[0].spritePath==recipes[0].path);
 require(regionSectionPath(recipes[0],1)=="Realms\\Celtic\\Swamp/CSsec01.map");
 for(const auto& bad:{source+"Specific=(1,0,0,0)\n",source+"[Region6]\n",std::string("[REGION0]\nName=A\n"),std::string("; empty\n")})require(std::holds_alternative<PersistenceError>(decodeRegionRecipes(bytes(bad))));
 std::cout<<"Owned region recipes, ordered lists, wildcards and duplicate rejection pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
