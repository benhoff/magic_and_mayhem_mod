#include "terrain_lighting.hpp"
#include "palette_lighting.hpp"
#include <algorithm>
#include <stdexcept>
static void require(bool v){if(!v)throw std::runtime_error("Terrain lighting assertion failed");}
int main(){using namespace mnm;
 const auto read=[](const std::string& text){auto c=assets::decodeConfig(assets::Bytes(text.begin(),text.end()));return assets::decodeTerrainLightingFields(std::get<assets::Config>(c));};
 const auto fields=std::get<assets::TerrainLightingFields>(read("[global_options]\nambientlight=+50 ; comment\nLIGHTRAMP=7\n"));require(fields.ambientLight==50 && fields.lightRamp==7);
 for(const auto& value:{"", "NaN", "7junk", "1.5", "2147483648"})require(std::holds_alternative<assets::PersistenceError>(read(std::string("[GLOBAL_OPTIONS]\nLightRamp=")+value+"\n")));
 require(!std::get<assets::TerrainLightingFields>(read("[OTHER]\nAmbientLight=5\n")).ambientLight);
 auto config=reconstruction::applyTerrainLighting({},999,-999);require(config.ambient==-127 && config.ramp==-127);
 config=reconstruction::applyTerrainLighting({},-999,999);require(config.ambient==127 && config.ramp==127);
 reconstruction::TerrainLightField field(32,32,5,{ -50,7 });require(field.buffers()[0].size()==32*32*3);require(field.kernels()[17][0]==-8);
 field.stamp({0,0,1,17});require(field.at(0,0,0)==-127);field.publish();require(field.at(0,0,0)==-8 && field.at(0,0,1)==-8);require(field.at(1,0,0)==field.at(31,0,0));require(field.at(0,1,0)==field.at(0,31,0));require(field.at(0,0,2)<field.at(0,0,0));require(field.buffers()[4][0]==-50);
 reconstruction::TerrainLightField rounding(18,19,5,{-50,9});require(rounding.kernels()[12][(11*12+1)*12+2]==-119);
 const auto prior=field.buffers();field.stamp({0,0,0,2});field.publish();require(field.buffers()==prior);
 const auto invalid=[&](reconstruction::TerrainLightSource s){bool refused=false;try{field.stamp(s);}catch(const std::invalid_argument&){refused=true;}require(refused && field.buffers()==prior);};
 invalid({32,0,0,17});invalid({0,32,0,17});invalid({0,0,5,17});invalid({0,0,0,1});invalid({0,0,0,18});
 bool refused=false;try{field.at(0,0,5);}catch(const std::out_of_range&){refused=true;}require(refused);
 reconstruction::TerrainLightField narrow(1,2,1,{-50,7});const auto narrowBefore=narrow.buffers();refused=false;try{narrow.stamp({0,0,0,2});}catch(const std::invalid_argument&){refused=true;}require(refused && narrow.buffers()==narrowBefore);
 auto copy=field;copy.stamp({16,16,4,17});copy.publish();require(field.buffers()==prior && copy.buffers()!=prior);
}
