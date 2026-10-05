// Serialize owned native palette tables on the host architecture for the oracle.
#include "palette_shading.hpp"
#include <fstream>
#include <iterator>
#include <cstring>
#include <stdexcept>
int main(int argc,char** argv){
 if(argc!=4 && argc!=8)throw std::runtime_error("SPR count output expected");
 std::ifstream in(argv[1],std::ios::binary);std::vector<unsigned char> b(std::istreambuf_iterator<char>(in),{});
 if(b.size()<24)throw std::runtime_error("SPR header");unsigned palettes=0;std::memcpy(&palettes,b.data()+16,4);
 if(!palettes || palettes>4 || b.size()<24+768*palettes)throw std::runtime_error("Palette bounds");
 mnm::reconstruction::PaletteShadingConfig config{unsigned(std::stoul(argv[2])),1,1,2,2};
 if(argc==8){config.intensityLevel=std::stoi(argv[4]);config.saturationLevel=std::stoi(argv[5]);config.intensityPower=std::stod(argv[6]);config.saturationPower=std::stod(argv[7]);}
 std::ofstream out(argv[3],std::ios::binary);
 for(unsigned i=0;i<palettes;++i){mnm::reconstruction::PaletteRgb rgb{};std::memcpy(rgb.data(),b.data()+24+768*i,768);
  for(bool format:{false,true}){const auto p=mnm::reconstruction::buildShadedPalette(rgb,config,format);
   for(const auto& table:p.tables)out.write(reinterpret_cast<const char*>(table.data()),512);}}
 if(!out)throw std::runtime_error("Native palette output");
}
