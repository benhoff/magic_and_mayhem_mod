#include "effect_animation_catalog.hpp"
#include <charconv>
#include <stdexcept>

namespace mnm::assets {
namespace {
std::uint32_t decimal(const std::string& value){
    std::uint32_t n=0;const auto r=std::from_chars(value.data(),value.data()+value.size(),n);
    if(r.ec!=std::errc{} || r.ptr!=value.data()+value.size())
        throw std::invalid_argument("Invalid effect animation unsigned decimal: "+value);
    return n;
}
std::string upper(std::string value){for(auto& c:value)if(c>='a' && c<='z')c=char(c-'a'+'A');return value;}
std::unique_ptr<AssetFile> open(const AssetStore& store,const std::string& path){
    auto result=store.open(path);
    if(const auto* e=std::get_if<Error>(&result))throw std::runtime_error(path+": "+e->detail);
    return std::get<std::unique_ptr<AssetFile>>(std::move(result));
}
}
EffectAnimationRecipes readEffectAnimationRecipes(const Config& config,const EffectAnimationCatalogLimits& limits){
    auto field=[&](const std::string& section,const char* key){const auto* value=config.find(section,key);
        auto s=value?value->substr(0,value->find(';')):std::string{};
        const auto start=s.find_first_not_of(" \t\r");
        s=start==s.npos?std::string{}:s.substr(start,s.find_last_not_of(" \t\r")-start+1);
        if(s.size()>=2 && (s.front()=='\"' || s.front()=='\'') && s.back()==s.front())s=s.substr(1,s.size()-2);
        if(s.empty())throw std::invalid_argument("Missing effect animation field: "+section+"/"+key);
        return s;};
    const auto count=decimal(field("HEADER","NumberofAnimations"));
    const auto files=decimal(field("HEADER","NumberOfEffectsFile"));
    if(count==0 || count>limits.entries || files==0 || files>limits.files)
        throw std::invalid_argument("Effect animation catalog capacity");
    EffectAnimationRecipes result{files,{}};result.entries.reserve(count);
    for(std::uint32_t i=0;i<count;++i){
        const auto section="ANI_"+std::to_string(i);
        const auto reference=upper(field(section,"AnimationFileRef"));
        std::optional<std::uint32_t> asset;
        // Exact generated EFFECTS<number> match, rather than prefix parsing.
        for(std::uint32_t j=0;j<files;++j)if(reference=="EFFECTS"+std::to_string(j))asset=j;
        if(!asset)throw std::invalid_argument("Unknown effect ANI reference: "+reference);
        const auto printer=upper(field(section,"SpritePrinter"));std::uint32_t code=0;
        if(printer=="NORMAL")code=31;
        else if(printer=="TRANSPARENT")code=2;
        else if(printer=="TRANSPARENT25")code=3;
        else if(printer=="TRANSPARENT50")code=4;
        else if(printer=="TRANSPARENT75")code=5;
        else throw std::invalid_argument("Unknown effect sprite printer: "+printer);
        result.entries.push_back({decimal(field(section,"AnimationNo")),*asset,code,decimal(field(section,"Data1"))});
    }
    return result;
}
EffectAnimationCatalog loadEffectAnimationCatalog(const AssetStore& store,const EffectAnimationCatalogLimits& limits){
    auto file=open(store,"CFG/Encrypted/effectani.cfg");PersistenceLimits packedLimits;
    packedLimits.inputBytes=packedLimits.decodedBytes=1024*1024;
    auto packed=loadConfig(*file,true,packedLimits);file.reset();
    if(const auto* e=std::get_if<PersistenceError>(&packed))throw std::runtime_error("effectani.cfg: "+e->detail);
    EffectAnimationCatalog result;
    result.recipes=readEffectAnimationRecipes(std::get<Config>(packed),limits);
    result.animations.reserve(result.recipes.fileCount);result.spritePaths.reserve(result.recipes.fileCount);
    for(std::uint32_t i=0;i<result.recipes.fileCount;++i){
        const auto base="Sprites/EFFECTS"+std::to_string(i);
        file=open(store,base+".ani");auto animation=loadAnimation(*file,limits.animation);file.reset();
        if(const auto* e=std::get_if<AnimationError>(&animation))throw std::runtime_error(base+".ani: "+e->detail);
        result.animations.push_back(std::get<Animation>(std::move(animation)));result.spritePaths.push_back(base+".spr");
    }
    for(const auto& entry:result.recipes.entries){
        const auto& animation=result.animations[entry.assetIndex];
        if(std::uint64_t(entry.sequence)+1>=animation.starts.size())
            throw std::invalid_argument("Effect catalog sequence outside loaded ANI");
    }
    return result;
}
}
