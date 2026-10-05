#include "light_fixture.hpp"
#include <charconv>
#include <stdexcept>
#include <string>
namespace mnm::preview {
namespace {
std::string word(std::istream& in){std::string text;if(!(in>>text) || text.size()>32)throw std::invalid_argument("Invalid or truncated lighting fixture token");return text;}
void expect(std::istream& in,const char* expected){if(word(in)!=expected)throw std::invalid_argument("Unexpected lighting fixture token");}
int integer(std::istream& in,int low,int high){const auto text=word(in);int value=0;const auto result=std::from_chars(text.data(),text.data()+text.size(),value);if(result.ec!=std::errc{} || result.ptr!=text.data()+text.size() || value<low || value>high)throw std::invalid_argument("Lighting fixture integer outside contract");return value;}
unsigned coordinate(std::istream& in){return unsigned(integer(in,0,127));}
}
std::vector<reconstruction::TerrainLightingSnapshot> readLightingFixture(std::istream& in){
 expect(in,"MNM_LIGHTING");if(integer(in,1,1)!=1)throw std::invalid_argument("Unsupported lighting fixture version");
 expect(in,"relations");reconstruction::TerrainLightRelations relations;
 for(auto* table:{&relations.first,&relations.second})for(auto& row:*table)for(auto& cell:row)cell=std::uint8_t(integer(in,0,255));
 std::vector<reconstruction::TerrainLightingSnapshot> snapshots;
 for(auto token=word(in);token!="end";token=word(in)){
  if(token!="tick" || snapshots.size()>=64)throw std::invalid_argument("Lighting fixture requires 1..64 tick snapshots");
  reconstruction::TerrainLightingSnapshot s;s.relations=relations;
  s.player=integer(in,0,7);s.changed=integer(in,-2147483647-1,2147483647);
  s.creaturesEnabled=integer(in,0,1)!=0;s.objectsEnabled=integer(in,0,1)!=0;
  s.view={coordinate(in),coordinate(in),unsigned(integer(in,0,128))};
  s.creatureScan=unsigned(integer(in,0,260));s.objectScan=unsigned(integer(in,0,256));
  const auto creatures=integer(in,0,256),objects=integer(in,0,256);
  if(s.objectScan>unsigned(objects))throw std::invalid_argument("Lighting object scan exceeds capacity");
  for(int i=0;i<creatures;++i){expect(in,"creature");reconstruction::TerrainLightCreature c;
   c.active=integer(in,0,1)!=0;c.lightEnabled=integer(in,0,1)!=0;c.owner=integer(in,0,7);
   c.column=coordinate(in);c.row=coordinate(in);c.layer=unsigned(integer(in,0,31));s.creatures.push_back(c);
  }
  for(int i=0;i<objects;++i){expect(in,"object");reconstruction::TerrainLightObject o;
   o.active=integer(in,0,1)!=0;o.lightIndex=unsigned(integer(in,0,33));
   if(o.lightIndex==1)throw std::invalid_argument("Special size-one static light is unsupported");
   o.column=coordinate(in);o.row=coordinate(in);o.layer=unsigned(integer(in,0,31));
   o.admissionColumn=coordinate(in);o.admissionRow=coordinate(in);s.objects.push_back(o);
  }
  snapshots.push_back(std::move(s));
 }
 std::string trailing;if(snapshots.empty() || in>>trailing)throw std::invalid_argument("Lighting fixture empty or has trailing data");
 return snapshots;
}
}
