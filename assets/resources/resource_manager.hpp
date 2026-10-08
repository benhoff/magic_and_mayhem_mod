#pragma once
#include "../animation.hpp"
#include "../bmp.hpp"
#include "../jpeg.hpp"
#include "../pcx.hpp"
#include "../sprite_loader.hpp"
#include "../terrain_catalog.hpp"
#include <map>
#include <thread>
#include <functional>

namespace mnm::assets {
enum class ResourceKind { creature, terrain, effect, ui };
// Caller-assigned semantic names, independent of load order, pointers and paths.
// Names use lowercase ASCII [a-z0-9_-] components separated by '/'.
struct ResourceId {
    ResourceKind kind=ResourceKind::ui;
    std::string name;
    bool operator<(const ResourceId&) const;
    bool operator==(const ResourceId&) const;
    std::string text() const;
};
enum class ResourceImageFormat { sprite, bmp, pcx, jpeg };
struct ResourceRecipe {
    ResourceImageFormat format=ResourceImageFormat::sprite;
    std::string image;
    std::optional<std::string> animation, terrainCatalog;
    std::optional<std::uint32_t> sequence;
    // Owned checkpoint ANI source, mutually exclusive with the ANI path.
    std::optional<std::vector<std::uint8_t>> animationBytes{};
    bool operator==(const ResourceRecipe&) const;
};
using ResourceImage=std::variant<Sprite,BmpImage,PcxImage,JpegImage>;
struct VisualResource {
    ResourceImage image;
    std::optional<Animation> animation;
    std::optional<TerrainCatalog> terrainCatalog;
    std::uint64_t revision=0, decodedBytes=0;
    std::size_t frameCount() const;
};
struct ResourceLimits {
    std::size_t bindings=1024, residentResources=64;
    // Owned vector capacities + object storage, excluding allocator/map overhead.
    // Decoder scratch/input allocations have separate existing loader bounds.
    std::uint64_t decodedBytes=128ULL*1024*1024;
    std::uint64_t recipeBytes=8ULL*1024*1024;
    SpriteLimits sprite;
    AnimationLimits animation;
    BmpLimits bmp;
    PcxLimits pcx;
    JpegLimits jpeg;
};
struct ResourceStats {
    std::size_t bindings=0, residentResources=0;
    std::uint64_t decodedBytes=0, loads=0, hits=0;
    std::uint64_t recipeBytes=0;
};
class PreparedResource;
// Immutable, owned request. Its private binding token makes late completions
// inadmissible after unload, or in another ResourceManager.
class ResourcePreparation final {
public:
    const ResourceId& id() const { return id_; }
private:
    friend class ResourceManager;
    friend PreparedResource prepareResource(const ResourcePreparation&,const std::function<void()>&,const std::string&);
    ResourcePreparation(AssetStore store,ResourceId id,ResourceRecipe recipe,ResourceLimits limits,
                        std::shared_ptr<const int> binding)
        :store_(std::move(store)),id_(std::move(id)),recipe_(std::move(recipe)),limits_(limits),binding_(std::move(binding)){}
    AssetStore store_;
    ResourceId id_;
    ResourceRecipe recipe_;
    ResourceLimits limits_;
    std::shared_ptr<const int> binding_;
};
class PreparedResource final {
public:
    PreparedResource(PreparedResource&&)=default;
    PreparedResource& operator=(PreparedResource&&)=default;
    const ResourceId& id() const { return request_.id(); }
    const VisualResource& resource() const { return *resource_; }
    const std::vector<std::uint8_t>& sourceImage() const { return source_; }
    void discardSource() { std::vector<std::uint8_t>().swap(source_); }
private:
    friend class ResourceManager;
    friend PreparedResource prepareResource(const ResourcePreparation&,const std::function<void()>&,const std::string&);
    explicit PreparedResource(ResourcePreparation request):request_(std::move(request)){}
    ResourcePreparation request_;
    std::unique_ptr<VisualResource> resource_;
    std::vector<std::uint8_t> source_;
};
// CPU-only preparation; checkpoint may cancel by throwing. No manager, widget,
// file handle or GL object is borrowed. Encoded image remains owned until adoption.
PreparedResource prepareResource(const ResourcePreparation&,const std::function<void()>& checkpoint={},const std::string& expectedImageSha256={});
// Thread-confined, read-only input. Immutable bindings; unload releases decoded
// data but preserves the ID/recipe. Each subsequent successful load gets a fresh
// revision, so GPU caches never reuse a retired image. No implicit file watching.
// Returned references live until unload(id), unloadAll(), or destruction.
class ResourceManager final {
public:
    explicit ResourceManager(AssetStore store,ResourceLimits limits={});
    ResourceManager(const ResourceManager&)=delete;
    ResourceManager& operator=(const ResourceManager&)=delete;
    void bind(const ResourceId&,const ResourceRecipe&);
    const ResourceRecipe& recipe(const ResourceId&) const;
    const VisualResource& load(const ResourceId&);
    ResourcePreparation request(const ResourceId&);
    const VisualResource& adopt(PreparedResource&&);
    bool isResident(const ResourceId&) const;
    const VisualResource& resident(const ResourceId&) const;
    void unload(const ResourceId&);
    void unloadAll();
    ResourceStats stats() const;
private:
    struct Entry {ResourceRecipe recipe;std::unique_ptr<VisualResource> resource;std::shared_ptr<const int> binding;};
    AssetStore store_;
    ResourceLimits limits_;
    std::thread::id thread_;
    std::map<ResourceId,Entry> entries_;
    ResourceStats stats_;
    std::uint64_t nextRevision_=1;
    void checkThread() const;
    std::unique_ptr<AssetFile> open(const std::string&) const;
};
}
