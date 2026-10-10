#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <QImage>
#include <QSize>
class QOpenGLContext;

namespace mnm::render {
struct Image {
    int width=0, height=0;
    // One unsigned native pixel per word, including palette indices/unused bits.
    // Row zero is the logical top row. There is no row padding here.
    std::vector<std::uint32_t> pixels;
};
struct Rect {int left=0, top=0, right=0, bottom=0;};
struct ClipperState;
struct DibInput;
struct SurfaceCopyRequest;
struct SurfaceCopyResult;
struct Blit {
    Image source, destination;
    unsigned bits=0;
    Rect sourceRect;
    int destinationX=0, destinationY=0;
    std::optional<std::uint32_t> sourceKey;
};
void validate(const Blit& command);
struct Driver {std::string vendor, renderer, version;};
using SurfaceId=std::uint64_t;
struct PixelFormat {unsigned bits=0;std::array<std::uint32_t,3> masks{};};
enum class CompositeMode {copy,half,quarterSource,displace,quarterDestination,projectedShadow};
struct SpriteComposite {
    CompositeMode mode=CompositeMode::copy;
    // Nonnegative horizontal displacement, sampling untouched pixels to the right.
    std::array<int,16> rowOffsets{};
    unsigned rowPeriod=16;
};
struct Rgb {std::uint8_t red=0,green=0,blue=0;};
// GUI-thread lease of the latest GPU presentation. Copies share its texture.
// It survives source-surface and renderer destruction; subsequent presentations
// of that surface update existing leases. Consumers bracket draws with these
// methods in a sharing context, establishing GPU ordering in both directions.
class GpuFrame final {
public:
    GpuFrame()=default;
    QSize size() const;
    bool valid() const {return bool(data_);}
    unsigned textureForCurrentContext() const;
    void samplingComplete() const;
private:
    struct Data;
    std::shared_ptr<Data> data_;
    explicit GpuFrame(std::shared_ptr<Data> data):data_(std::move(data)){}
    friend class GlBlitter;
};
struct RenderStats {
    std::uint64_t uploads=0,copies=0,paletteUpdates=0,nativeReadbacks=0,presentations=0;
    std::uint64_t rgbaReadbacks=0,gpuPresentations=0;
    std::uint64_t presentationPixels=0;
    std::size_t surfaces=0,pixels=0;
};

// Qt GUI-thread owner of a dedicated OpenGL 3.3 context. No captured output is
// supplied to draw(): only source pixels, the old destination and the command.
class GlBlitter final {
public:
    explicit GlBlitter(QOpenGLContext* shareContext=nullptr);
    ~GlBlitter();
    GlBlitter(const GlBlitter&)=delete;
    GlBlitter& operator=(const GlBlitter&)=delete;
    Image draw(const Blit& command);
    Driver driver() const;
    // Synchronous renderer operations only: retain the dedicated context until
    // return, restoring the caller on success or exception. No event dispatch,
    // presentation callbacks or foreign GL state changes inside this scope.
    // Nested batches are allowed. Release builds check GL errors at the outer
    // boundary; allocation checks and Debug operation checks remain immediate.
    void batch(const std::function<void()>& operations);
    // Persistent, renderer-owned surfaces: max 2048x2048, 64 handles, 16M pixels.
    // Copies retain textures; read/present are explicit synchronization points.
    SurfaceId create(const Image& image,PixelFormat format);
    // Allocate undefined native storage without a CPU fill/upload. Sampling and
    // presentation remain forbidden until the written region is defined.
    SurfaceId allocate(int width,int height,PixelFormat format);
    // Upload contiguous full-width rows without copying the owned CPU plane.
    void updateRows(SurfaceId surface,const Image& plane,int first,int rows);
    void updateRegionRows(SurfaceId surface,int x,int y,const Image& plane,int first,int rows);
    void destroy(SurfaceId surface);
    void update(SurfaceId surface,int x,int y,const Image& patch);
    // SRCCOPY at origin to indexed8 (fully installed palette), RGB565 or RGB24/32.
    // Owned application DC regions: nullopt means no clip, empty means no writes.
    // Attached DirectDraw Blt clippers do not constrain this bitmap operation.
    // Only the cropped written rectangle becomes valid.
    void reloadDib(SurfaceId surface,const DibInput& dib,
                   const std::optional<std::vector<Rect>>& dcRegions=std::nullopt);
    void copy(SurfaceId source,SurfaceId destination,Rect rect,int x,int y,
              std::optional<std::uint32_t> key=std::nullopt,
              std::optional<SurfaceId> mask=std::nullopt,std::uint32_t keyMask=UINT32_MAX);
    void composite(SurfaceId source,SurfaceId destination,Rect rect,int x,int y,
                   SurfaceId mask,const SpriteComposite&,
                   const std::array<std::uint16_t,256>* palette=nullptr);
    void colourRect(SurfaceId,const Image&,Rect,int,int,const SpriteComposite&);
    void additiveRect(SurfaceId destination,Rect,const std::array<std::uint16_t,3>& channels);
    // Surface2-style same-format native operation; see surface_copy.hpp for the bounded policy.
    // Same-ID opaque copies freeze each ordered source piece on the GPU (max16MiB
    // transient storage). copy() retains its distinct-ID primitive contract.
    // External accepted Restore/loss invalidates native bytes without inventing
    // deterministic contents. Updates/opaque copies establish only written regions.
    // Reads/presentation/source sampling refuse undefined pixels. No driver Restore.
    void invalidateContents(SurfaceId surface);
    void setClipper(SurfaceId destination,const ClipperState& clipper);
    SurfaceCopyResult surfaceCopy(SurfaceId source,SurfaceId destination,
                                 const SurfaceCopyRequest& request,
                                 std::optional<std::uint32_t> nativeKey=std::nullopt);
    // Optional indexed8 mask has source dimensions; zero discards the pixel.
    // Masks use source coordinates, retain destination pixels and do not read back.
    // Exchange native storage only; palettes and handles retain their identity.
    void swapContents(SurfaceId first,SurfaceId second);
    void setPalette(SurfaceId surface,unsigned first,const std::vector<Rgb>& colors);
    Image read(SurfaceId surface);
    QImage present(SurfaceId surface);
    GpuFrame presentGpu(SurfaceId surface);
    RenderStats stats() const;
private:
    struct Impl;
    std::shared_ptr<Impl> impl_;
    friend class GpuFrame;
};
}
