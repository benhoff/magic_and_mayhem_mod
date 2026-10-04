#include "region_recipe.hpp"
#include "persistence_internal.hpp"
#include <charconv>
#include <sstream>
#include <set>
#include <iomanip>
namespace mnm::assets {
namespace {
using namespace persistence_detail;
std::string trim(std::string s){auto a=s.find_first_not_of(" \t\r\n");return a==s.npos?std::string{}:s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);}
std::string lower(std::string s){for(auto& c:s)if(c>='A' && c<='Z')c+=32;return s;}
struct List {
 const std::string& s;std::size_t at=0;
 void ws(){while(at<s.size() && (s[at]==' ' || s[at]=='\t'))++at;}
 bool end(){ws();return at==s.size();}
 void token(char c){ws();if(at==s.size() || s[at++]!=c)fail(PersistenceErrorCode::malformedData,at,"Region list delimiter");}
 int number(){ws();int v=0;auto r=std::from_chars(s.data()+at,s.data()+s.size(),v);if(r.ec!=std::errc{} || r.ptr==s.data()+at)fail(PersistenceErrorCode::malformedData,at,"Region decimal integer");at=r.ptr-s.data();return v;}
};
}
std::string regionSectionPath(const RegionRecipe& r,std::uint32_t section){std::ostringstream s;s<<r.path<<'/'<<r.prefix<<std::setfill('0')<<std::setw(2)<<section<<".map";return s.str();}
PersistenceResult<std::vector<RegionRecipe>> decodeRegionRecipes(const Bytes& input,const PersistenceLimits& limits){
 return guarded<std::vector<RegionRecipe>>([&]{
  cap(input.size(),limits.inputBytes);Bytes bytes=input;
  cap(bytes.size(),limits.decodedBytes);if(std::find(bytes.begin(),bytes.end(),0)!=bytes.end())fail(PersistenceErrorCode::malformedData,0,"NUL in region CFG");
  std::map<unsigned,std::map<std::string,std::string>> fields;std::optional<unsigned> active;
  std::istringstream lines(std::string(bytes.begin(),bytes.end()));std::string line;unsigned count=0;
  const std::set<std::string> keys{"name","path","spritepath","sectionprefix","mapsize","specific","random"};
  while(std::getline(lines,line)){
   cap(++count,limits.lines);line=trim(line.substr(0,line.find(';')));if(line.empty() || line.front()=='#')continue;
   if(line.front()=='['){active.reset();if(line.back()!=']')fail(PersistenceErrorCode::malformedData,0,"Region section header");auto name=lower(trim(line.substr(1,line.size()-2)));
    if(name.size()>6 && name.rfind("region",0)==0 && name[6]>='0' && name[6]<='9'){List n{name};n.at=6;const auto id=n.number();if(id<0 || id>99 || !n.end() || fields.count(id))fail(PersistenceErrorCode::malformedData,0,"Region section ID");active=unsigned(id);fields[*active];}continue;
   }
   if(!active)continue;
   const auto equal=line.find('=');if(equal==line.npos)continue;const auto key=lower(trim(line.substr(0,equal)));if(!keys.count(key))continue;
   if(!fields[*active].emplace(key,trim(line.substr(equal+1))).second)fail(PersistenceErrorCode::malformedData,0,"Duplicate region recipe key");
  }
  std::vector<RegionRecipe> result;
  for(const auto& entry:fields){const auto& f=entry.second;const auto required=[&](const char* key){const auto i=f.find(key);if(i==f.end())fail(PersistenceErrorCode::malformedData,0,std::string("Missing region ")+key);return i->second;};
   RegionRecipe r;r.id=entry.first;r.name=required("name");r.path=required("path");r.prefix=required("sectionprefix");r.spritePath=f.count("spritepath")?f.at("spritepath"):r.path;
   if(r.name.empty() || r.path.empty() || r.prefix.empty() || r.spritePath.empty() || r.prefix.find_first_of("/\\:")!=r.prefix.npos)fail(PersistenceErrorCode::malformedData,0,"Region path/prefix");
   const auto size=required("mapsize");List d{size};d.token('(');auto w=d.number();d.token(',');auto h=d.number();d.token(')');if(!d.end() || w<1 || w>5 || h<1 || h>5)fail(PersistenceErrorCode::malformedData,0,"Region grid capacity");r.columns=w;r.rows=h;
   const auto specific=required("specific");List s{specific};while(!s.end()){
    s.token('(');const auto id=s.number();s.token(',');const auto rotation=s.number();s.token(',');const auto x=s.number();s.token(',');const auto y=s.number();s.token(')');
    if(id<0 || id>99 || rotation< -1 || rotation>3 || x< -1 || x>=w || y< -1 || y>=h)fail(PersistenceErrorCode::malformedData,0,"Region specific fields");
    cap(r.specific.size()+1,50);r.specific.push_back({unsigned(id),rotation,x,y});
   }
   const auto random=f.count("random")?f.at("random"):std::string{};List q{random};if(!q.end()){q.token('(');while(true){const auto id=q.number();q.token('_');const auto n=q.number();if(id<0 || id>99 || n<1 || n>999)fail(PersistenceErrorCode::malformedData,0,"Region random fields");cap(r.random.size()+1,50);r.random.push_back({unsigned(id),unsigned(n)});q.ws();if(q.at<random.size() && random[q.at]==','){q.token(',');continue;}q.token(')');break;}if(!q.end())fail(PersistenceErrorCode::malformedData,0,"Region random trailing bytes");}
   result.push_back(std::move(r));
  }
  if(result.empty())fail(PersistenceErrorCode::malformedData,0,"No region recipes");
  return result;
 });
}
PersistenceResult<std::vector<RegionRecipe>> loadRegionRecipes(AssetFile& file,bool packed,const PersistenceLimits& limits){
 return persistence_detail::load<std::vector<RegionRecipe>>(file,limits,[&](const Bytes& bytes)->PersistenceResult<std::vector<RegionRecipe>>{
  if(!packed)return decodeRegionRecipes(bytes,limits);
  auto r=decodePackedContainer(bytes,limits);if(auto* e=std::get_if<PersistenceError>(&r))return *e;
  return decodeRegionRecipes(std::get<PackedContainer>(r).decoded,limits);
 });
}
}
