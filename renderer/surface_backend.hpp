#pragma once
#include "bitmap_dc.hpp"
#include "surface_access.hpp"
#include "pixel_rows.hpp"
#include <unordered_map>

namespace mnm::render {
// GUI-thread owned surfaces. No COM pointer, real HDC or shared-wire ownership.
class SurfaceBackend final {
public:
    using Palette=BitmapDcState::Palette;
    using PaletteId=std::uint64_t;
    struct Mode {int width,height;PixelFormat format;};
    using Descriptor=SurfaceAccessState::Descriptor;
    SurfaceBackend();
    ~SurfaceBackend();
    SurfaceBackend(const SurfaceBackend&)=delete;
    SurfaceBackend& operator=(const SurfaceBackend&)=delete;
    SurfaceId create(const Image&,PixelFormat,unsigned caps=0x840,
                     std::optional<Palette> defaults=std::nullopt,bool primary=false,
                     std::optional<std::int32_t> rowPitch=std::nullopt);
    SurfaceId createRows(const PixelRows&,PixelFormat,unsigned caps=0x840,
                         std::optional<Palette> defaults=std::nullopt);
    void updateRows(SurfaceId,int x,int y,const PixelRows&);
    void readRows(SurfaceId,PixelRows&);
    void writeLockedRows(SurfaceId,int x,int y,const PixelRows&);
    void destroy(SurfaceId);
    // Canonical aliases retain one object identity; IDs never recycle.
    SurfaceId alias(SurfaceId);
    unsigned release(SurfaceId);
    unsigned references(SurfaceId) const;
    PaletteId createPalette(const Palette&);
    unsigned retainPalette(PaletteId);
    unsigned releasePalette(PaletteId);
    void bindPaletteObject(SurfaceId,std::optional<PaletteId>);
    void updatePaletteObject(PaletteId,unsigned,const std::vector<Rgb>&);
    std::uint32_t getPaletteObject(SurfaceId,PaletteId&);
    std::optional<PaletteId> paletteIdentity(SurfaceId) const;
    std::size_t paletteObjects() const;
    void attachBackBuffer(SurfaceId front,SurfaceId back);
    SurfaceId getBackBuffer(SurfaceId front);
    std::uint32_t flip(SurfaceId front,std::uint32_t flags=1);
    void markLost(SurfaceId);
    std::uint32_t isLost(SurfaceId) const;
    std::uint32_t restore(SurfaceId,std::optional<Mode> currentMode=std::nullopt);
    std::optional<std::uint32_t> sourceKey(SurfaceId) const;
    void update(SurfaceId,int x,int y,const Image&);
    std::uint32_t lock(SurfaceId,Descriptor&);
    const Image& lockedPixels(SurfaceId) const;
    void writeLocked(SurfaceId,int x,int y,const Image&);
    std::uint32_t unlock(SurfaceId);
    std::uint32_t acquireDc(SurfaceId,std::uint32_t& output);
    std::uint32_t releaseDc(SurfaceId,std::uint32_t token);
    const Palette& dcPalette(SurfaceId) const;
    const std::optional<std::vector<Rect>>& dcRegions(SurfaceId) const;
    void selectDcRegions(SurfaceId,std::optional<std::vector<Rect>>);
    void setDcColors(SurfaceId,unsigned first,const std::vector<Rgb>&);
    void bindPalette(SurfaceId,std::optional<Palette>);
    void updatePalette(SurfaceId,unsigned first,const std::vector<Rgb>&);
    void reloadDib(SurfaceId,const DibInput&);
    void setClipper(SurfaceId,const ClipperState&);
    void setSourceKey(SurfaceId,std::optional<std::uint32_t>);
    SurfaceCopyResult copy(SurfaceId source,SurfaceId destination,const SurfaceCopyRequest&);
    // API native color truncates to active RGB masks (indexed depth); the recovered WORD wrapper
    // performs its own truncation before this API. COLORFILL optionally WAIT.
    SurfaceCopyResult fill(SurfaceId,std::optional<Rect>,std::uint32_t color,
                           std::uint32_t flags=0x01000400);
    Image read(SurfaceId);
    QImage present(SurfaceId);
    GpuFrame presentGpu(SurfaceId);
    Image readDc(SurfaceId);
    QImage presentDc(SurfaceId);
    void invalidateContents(SurfaceId);
    bool borrowed(SurfaceId) const;
    bool poisoned(SurfaceId) const;
    int mapBalance(SurfaceId) const;
    RenderStats stats() const;
    Driver driver() const;
private:
    struct Entry;
    struct PaletteObject;
    GlBlitter gl_;
    std::unordered_map<SurfaceId,std::unique_ptr<Entry>> entries_;
    std::unordered_map<PaletteId,std::shared_ptr<PaletteObject>> palettes_;
    std::uint32_t nextToken_=1;
    Entry& entry(SurfaceId);
    const Entry& entry(SurfaceId) const;
    static void idle(const Entry&);
    static void available(const Entry&);
    void retire(SurfaceId);
    std::shared_ptr<PaletteObject> paletteObject(PaletteId) const;
    void propagatePalette(const std::shared_ptr<PaletteObject>&,unsigned,const std::vector<Rgb>&);
    static void raster(const Entry&);
    void installBinding(SurfaceId,const Entry&);
};
}
