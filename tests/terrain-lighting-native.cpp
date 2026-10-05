// Independent 64-bit serialization of owned fields/kernels for the PE32 runner.
#include "terrain_lighting.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
int main(int argc,char** argv)try{
 if(argc!=8)throw std::runtime_error("Expected width height layers ambient ramp sources prefix");
 const auto value=[](const char* text){return std::string(text)=="missing"?std::nullopt:std::optional<int>(std::stoi(text));};
 mnm::reconstruction::TerrainLightField field(std::stoul(argv[1]),std::stoul(argv[2]),std::stoul(argv[3]),mnm::reconstruction::applyTerrainLighting({},value(argv[4]),value(argv[5])));
 std::istringstream sources(argv[6]);std::string source;
 while(std::getline(sources,source,';')){if(source=="none")continue;std::replace(source.begin(),source.end(),',',' ');std::istringstream fields(source);mnm::reconstruction::TerrainLightSource s;if(!(fields>>s.column>>s.row>>s.layer>>s.size))throw std::runtime_error("Source syntax");field.stamp(s);}
 field.publish();std::ofstream kernels(std::string(argv[7])+".kernels",std::ios::binary);for(unsigned n=2;n<=17;++n){const auto& k=field.kernels()[n];kernels.write(reinterpret_cast<const char*>(k.data()),k.size());}
 std::ofstream buffers(std::string(argv[7])+".buffers",std::ios::binary);for(const auto& b:field.buffers())buffers.write(reinterpret_cast<const char*>(b.data()),b.size());if(!kernels || !buffers)throw std::runtime_error("Output failed");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
