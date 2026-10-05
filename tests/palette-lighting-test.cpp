#include "palette_lighting.hpp"
#include "palette_shading.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>
static void require(bool value){if(!value)throw std::runtime_error("Palette lighting test failed");}
int main(){using namespace mnm;
 auto decoded=assets::decodeConfig(assets::Bytes{'[','G','L','O','B','A','L','_','O','P','T','I','O','N','S',']','\n'});require(std::holds_alternative<assets::Config>(decoded));
 const auto read=[](const std::string& text){auto c=assets::decodeConfig(assets::Bytes(text.begin(),text.end()));if(auto* e=std::get_if<assets::PersistenceError>(&c))return assets::PersistenceResult<assets::PaletteLightingFields>(*e);return assets::decodePaletteLightingFields(std::get<assets::Config>(c));};
 auto r=read("[global_options]\nLIGHTcurve=+50 ; comment\nColourFactor=50\nLightPower=2.2\nColourPower=2.2\nIgnored=bad\n");require(std::holds_alternative<assets::PaletteLightingFields>(r));const auto& f=std::get<assets::PaletteLightingFields>(r);require(f.lightCurve==50 && f.colourFactor==50 && f.lightPower==2.2 && f.colourPower==2.2);
 auto c=reconstruction::applyPaletteLighting({}, {f.lightCurve,f.colourFactor,f.lightPower,f.colourPower});require(c.intensityLevel==50 && c.saturationLevel==50 && c.intensityPower==2.2 && c.saturationPower==2.2);
 auto absent=std::get<assets::PaletteLightingFields>(read("[OTHER]\nLightCurve=99\n"));require(!absent.lightCurve && !absent.lightPower);auto unchanged=reconstruction::applyPaletteLighting(c,{});require(unchanged.intensityLevel==50 && unchanged.intensityPower==2.2);
 c=reconstruction::applyPaletteLighting({}, {-1,2000,-2.,2000.});require(c.intensityLevel==1 && c.saturationLevel==1000 && c.intensityPower==0 && c.saturationPower==1000);
 for(const auto& value:{"nan","inf","2.2trailing","","1e9999"})require(std::holds_alternative<assets::PersistenceError>(read(std::string("[GLOBAL_OPTIONS]\nLightPower=")+value+"\n")));
 for(const auto& value:{"1.5","2147483648","bad",""})require(std::holds_alternative<assets::PersistenceError>(read(std::string("[GLOBAL_OPTIONS]\nLightCurve=")+value+"\n")));
 bool refused=false;try{reconstruction::applyPaletteLighting({},{{},{},std::numeric_limits<double>::quiet_NaN(),{}});}catch(const std::invalid_argument&){refused=true;}require(refused);
 const auto preference=[](const std::string& text){auto c=assets::decodeConfig(assets::Bytes(text.begin(),text.end()));return assets::decodeTerrainPalettePreference(std::get<assets::Config>(c));};
 require(std::get<assets::TerrainPalettePreference>(preference("[video]\nterrainlightlevels=+256 ; installed\n")).lightLevels==256);
 require(!std::get<assets::TerrainPalettePreference>(preference("[OTHER]\nTerrainLightLevels=32\n")).lightLevels);
 for(const auto& v:{"", "1.5", "bad", "2147483648", "32junk"})require(std::holds_alternative<assets::PersistenceError>(preference(std::string("[VIDEO]\nTerrainLightLevels=")+v+"\n")));
 require(reconstruction::applyTerrainPalettePreference({},{}).count==16);
 require(reconstruction::applyTerrainPalettePreference({},-999).count==2);
 require(reconstruction::applyTerrainPalettePreference({},999).count==256);
 require(reconstruction::applyTerrainPalettePreference({},17).count==17);
 reconstruction::PaletteRgb rgb{};rgb[0]={255,255,255};const auto zero=reconstruction::buildShadedPalette(rgb,{16,1000,1,0,0});require(zero.tables.size()==15);
 refused=false;try{reconstruction::buildShadedPalette(rgb,{16,1000,1000,1000,1000});}catch(const std::invalid_argument&){refused=true;}require(refused);
}
