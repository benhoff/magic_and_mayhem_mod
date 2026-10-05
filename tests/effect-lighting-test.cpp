#include "effect_lighting.hpp"
#include "../assets/effect_lighting.hpp"
#include "terrain_lighting_cycle.hpp"
#include <stdexcept>
using namespace mnm;
static void require(bool v){static unsigned check=0;++check;if(!v)throw std::runtime_error("Effect lighting assertion failed: "+std::to_string(check));}
int main(){
 const std::string text="[fxa_3]\nLIGHTsourceDiameter='4junk'\nLightSourceAffected=tRuE\nClippedToHeight=FALSE\n[FXA_88]\nLightSourceDiameter="+std::string(300,'0')+"\n";
 auto fields=assets::readEffectLightingFields(assets::ProfileSnapshot::parse(assets::Bytes(text.begin(),text.end())));
 require(fields.values[3][0]=="4junk" && fields.values[88][0].size()==255 && fields.values[0][0].empty());
 reconstruction::EffectLightingTable table;table.reload(fields.values);
 require(table.lightIndex(3)==4 && table.entries()[3].affected==1 && table.entries()[3].clippedToHeight==0);
 fields.values[3]={"","FALSE","true"};table.reload(fields.values);
 require(table.lightIndex(3)==4 && table.entries()[3].affected==1 && table.entries()[3].clippedToHeight==1);
 auto old=table.entries();fields.values[0][0]="33";fields.values[88][0]=std::string(256,'1');
 bool refused=false;try{table.reload(fields.values);}catch(const std::invalid_argument&){refused=true;}require(refused && table.lightIndex(0)==old[0].diameter);
 refused=false;try{table.lightIndex(89);}catch(const std::out_of_range&){refused=true;}require(refused);
 reconstruction::TerrainLightingCycle cycle(32,32,3,{-50,7});reconstruction::TerrainLightingSnapshot snapshot;
 snapshot.changed=1;snapshot.objectsEnabled=true;snapshot.view={8,8,16};snapshot.objectScan=1;
 snapshot.objects.push_back(table.source(3,{true,0,8,8,0,8,8}));cycle.step(snapshot);
 require(cycle.field().at(8,8,0)==-85 && snapshot.objects[0].lightIndex==4);
 fields={};fields.values[3][0]="1";table.reload(fields.values);require(table.lightIndex(3)==1);
 snapshot.objects[0]=table.source(3,snapshot.objects[0]);refused=false;try{cycle.step(snapshot);}catch(const std::invalid_argument&){refused=true;}require(refused && cycle.ticks()==1);
}
