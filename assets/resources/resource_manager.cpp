#include "resource_manager.hpp"
#include <limits>
#include <stdexcept>
#include <tuple>

namespace mnm::assets {
namespace {
const char* kindName(ResourceKind kind){
    switch(kind){case ResourceKind::creature:return "creature";case ResourceKind::terrain:return "terrain";
    case ResourceKind::effect:return "effect";case ResourceKind::ui:return "ui";}
    throw std::invalid_argument("Unknown resource kind");
}
void validateId(const ResourceId& id){
    kindName(id.kind);
    if(id.name.empty() || id.name.size()>128)throw std::invalid_argument("Resource name must contain 1..128 bytes");
    bool component=false;
    for(const auto c:id.name){
        if(c=='/'){if(!component)throw std::invalid_argument("Empty resource name component");component=false;}
        else if((c>='a' && c<='z') || (c>='0' && c<='9') || c=='_' || c=='-')component=true;
        else throw std::invalid_argument("Resource name is not canonical lowercase ASCII");
    }
    if(!component)throw std::invalid_argument("Empty resource name component");
}
void validateRecipe(const ResourceId& id,const ResourceRecipe& r){
    validateId(id);
    const auto path=[](const std::string& p){
        if(p.empty() || p.size()>1024 || p.find('\0')!=std::string::npos)
            throw std::invalid_argument("Resource path must contain 1..1024 non-NUL bytes");
    };
    path(r.image);if(r.animation)path(*r.animation);if(r.terrainCatalog)path(*r.terrainCatalog);
    switch(r.format){case ResourceImageFormat::sprite:case ResourceImageFormat::bmp:
    case ResourceImageFormat::pcx:case ResourceImageFormat::jpeg:break;
    default:throw std::invalid_argument("Unknown resource image format");}
    if(r.animation && r.animationBytes)throw std::invalid_argument("ANI path and owned ANI bytes are mutually exclusive");
    const bool animated=bool(r.animation) || bool(r.animationBytes);
    if((animated || r.terrainCatalog) && r.format!=ResourceImageFormat::sprite)
        throw std::invalid_argument("ANI/TTD bindings require SPR input");
    if(r.sequence && !animated)throw std::invalid_argument("Sequence requires ANI input");
    if((id.kind==ResourceKind::creature || id.kind==ResourceKind::effect) && (!animated || r.terrainCatalog))
        throw std::invalid_argument("Creature/effect recipe requires ANI and excludes TTD");
    if(id.kind==ResourceKind::terrain && (!r.terrainCatalog || animated))
        throw std::invalid_argument("Terrain recipe requires TTD and excludes ANI");
    if(id.kind==ResourceKind::ui && r.terrainCatalog)throw std::invalid_argument("UI recipe excludes TTD");
}
template<class T,class E> T take(std::variant<T,E> result,const std::string& path){
    if(const auto* error=std::get_if<E>(&result))throw std::runtime_error(path+": "+error->detail);
    return std::get<T>(std::move(result));
}
template<class T> std::uint64_t storage(const std::vector<T>& v){return std::uint64_t(v.capacity())*sizeof(T);}
std::uint64_t storage(const Sprite& s){
    auto bytes=storage(s.palettes)+storage(s.frames);
    for(const auto& f:s.frames){
        bytes+=std::visit([](const auto& p){return storage(p);},f.pixels)+storage(f.opaqueMask);
        for(const auto& a:f.auxiliaryData)bytes+=storage(a);
    }
    return bytes;
}
std::uint64_t storage(const BmpImage& b){return storage(b.rgb);}
std::uint64_t storage(const PcxImage& p){return storage(p.indices);}
std::uint64_t storage(const JpegImage& j){return storage(j.rgb);}
std::uint64_t footprint(const VisualResource& r){
    auto bytes=sizeof(VisualResource)+std::visit([](const auto& image){return storage(image);},r.image);
    if(r.animation)bytes+=storage(r.animation->starts)+storage(r.animation->records);
    if(r.terrainCatalog)bytes+=storage(r.terrainCatalog->records);
    return bytes;
}
}
bool ResourceId::operator<(const ResourceId& other) const{return std::tie(kind,name)<std::tie(other.kind,other.name);}
bool ResourceId::operator==(const ResourceId& other) const{return kind==other.kind && name==other.name;}
std::string ResourceId::text() const{validateId(*this);return std::string(kindName(kind))+":"+name;}
bool ResourceRecipe::operator==(const ResourceRecipe& r) const{
    return std::tie(format,image,animation,terrainCatalog,sequence,animationBytes)==std::tie(r.format,r.image,r.animation,r.terrainCatalog,r.sequence,r.animationBytes);
}
std::size_t VisualResource::frameCount() const{
    if(const auto* s=std::get_if<Sprite>(&image))return s->frames.size();
    return 1;
}
ResourceManager::ResourceManager(AssetStore store,ResourceLimits limits)
    :store_(std::move(store)),limits_(limits),thread_(std::this_thread::get_id()){
    if(!limits.bindings || !limits.residentResources || !limits.decodedBytes || !limits.recipeBytes)
        throw std::invalid_argument("Resource budgets must be positive");
}
void ResourceManager::checkThread() const{
    if(std::this_thread::get_id()!=thread_)throw std::runtime_error("ResourceManager used from another thread");
}
void ResourceManager::bind(const ResourceId& id,const ResourceRecipe& recipe){
    checkThread();validateRecipe(id,recipe);
    const auto found=entries_.find(id);
    if(found!=entries_.end()){
        if(!(found->second.recipe==recipe))throw std::invalid_argument("Conflicting immutable resource binding: "+id.text());
        return;
    }
    if(entries_.size()>=limits_.bindings)throw std::runtime_error("Resource binding budget exceeded");
    const auto bytes=recipe.animationBytes?recipe.animationBytes->size():0;
    if(bytes>limits_.animation.inputBytes || bytes>limits_.recipeBytes-stats_.recipeBytes)
        throw std::runtime_error("Owned ANI recipe byte budget exceeded");
    Entry entry{recipe,{}};
    const auto capacity=entry.recipe.animationBytes?storage(*entry.recipe.animationBytes):0;
    if(capacity>limits_.recipeBytes-stats_.recipeBytes)throw std::runtime_error("Owned ANI recipe capacity budget exceeded");
    entries_.emplace(id,std::move(entry));stats_.recipeBytes+=capacity;stats_.bindings=entries_.size();
}
const ResourceRecipe& ResourceManager::recipe(const ResourceId& id) const{
    checkThread();validateId(id);return entries_.at(id).recipe;
}
std::unique_ptr<AssetFile> ResourceManager::open(const std::string& path) const{
    return take(store_.open(path),path);
}
const VisualResource& ResourceManager::load(const ResourceId& id){
    checkThread();validateId(id);auto& entry=entries_.at(id);
    if(entry.resource){++stats_.hits;return *entry.resource;}
    if(stats_.residentResources>=limits_.residentResources)throw std::runtime_error("Decoded resource count budget exceeded");
    if(nextRevision_==std::numeric_limits<std::uint64_t>::max())throw std::runtime_error("Resource revision exhausted");
    auto result=std::make_unique<VisualResource>();const auto& r=entry.recipe;
    {
        auto input=open(r.image);
        switch(r.format){
        case ResourceImageFormat::sprite:result->image=take(loadSprite(*input,limits_.sprite),r.image);break;
        case ResourceImageFormat::bmp:result->image=take(loadBmp(*input,limits_.bmp),r.image);break;
        case ResourceImageFormat::pcx:result->image=take(loadPcx(*input,limits_.pcx),r.image);break;
        case ResourceImageFormat::jpeg:result->image=take(loadJpeg(*input,limits_.jpeg),r.image);break;
        }
    }
    if(r.animation || r.animationBytes){
        const auto path=r.animation?*r.animation:id.text()+" owned ANI";
        if(r.animation){auto input=open(*r.animation);result->animation=take(loadAnimation(*input,limits_.animation),path);}
        else result->animation=take(decodeAnimation(*r.animationBytes,limits_.animation),path);
        for(const auto& record:result->animation->records)
            if(record.opcode==0 && (record.argument<0 || std::uint32_t(record.argument)>=result->frameCount()))
                throw std::runtime_error(path+": bitmap record outside paired SPR");
        if(r.sequence && *r.sequence>=result->animation->starts.size()-1)
            throw std::runtime_error(path+": selected sequence outside ANI");
    }
    if(r.terrainCatalog){auto input=open(*r.terrainCatalog);result->terrainCatalog=take(loadTerrainCatalog(*input),*r.terrainCatalog);}
    result->decodedBytes=footprint(*result);
    if(result->decodedBytes>limits_.decodedBytes-stats_.decodedBytes)
        throw std::runtime_error("Decoded resource byte budget exceeded: "+id.text());
    result->revision=nextRevision_++;
    stats_.decodedBytes+=result->decodedBytes;++stats_.residentResources;++stats_.loads;
    entry.resource=std::move(result);return *entry.resource;
}
void ResourceManager::unload(const ResourceId& id){
    checkThread();validateId(id);auto& entry=entries_.at(id);
    if(entry.resource){stats_.decodedBytes-=entry.resource->decodedBytes;--stats_.residentResources;entry.resource.reset();}
}
void ResourceManager::unloadAll(){
    checkThread();for(auto& item:entries_)item.second.resource.reset();stats_.decodedBytes=0;stats_.residentResources=0;
}
ResourceStats ResourceManager::stats() const{checkThread();return stats_;}
}
