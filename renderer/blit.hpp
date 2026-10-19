#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <QImage>
#include <QByteArray>
#include <unordered_map>
#include <QSize>
class QOpenGLContext;
class QOffscreenSurface;

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
struct GpuTiming {std::uint64_t sequence=0;double timelineMs=0;};
using SurfaceId=std::uint64_t;
struct PixelFormat {unsigned bits=0;std::array<std::uint32_t,3> masks{};};
enum class CompositeMode {copy,half,quarterSource,displace,quarterDestination,projectedShadow};
struct SpriteComposite {
    CompositeMode mode=CompositeMode::copy;
    // Nonnegative horizontal displacement, sampling untouched pixels to the right.
    std::array<int,16> rowOffsets{};
    unsigned rowPeriod=16;
    // Native finite-canvas policy: leave a covered pixel unchanged when its
    // displacement tap is beyond owned storage. Strict admission is the default.
    bool retainOutsideDisplacement=false;
};
struct Rgb {std::uint8_t red=0,green=0,blue=0;};
// Owner-thread lease of mutable GPU presentation. Copies share its texture.
// It survives source-surface and renderer destruction; subsequent presentations
// of that surface update existing leases. Isolated recovery frames instead
// own raster pixels and may retire on any host thread. Consumers bracket GPU draws with these
// methods in a sharing context, establishing GPU ordering in both directions.
// snapshot() exports immutable cross-thread pixels under an explicit owner-held
// lease; the render owner must reclaim the final reference before context teardown.
class GpuFrame final {
public:
    GpuFrame()=default;
    QSize size() const;
    bool valid() const {return bool(data_);}
    bool unique() const {return data_ && data_.use_count()==1;}
    // Immutable presentation copy. Keep a lease on the render owner until all
    // consumer leases return; final destruction must occur on that owner.
    GpuFrame snapshot(GpuFrame reusable={}) const;
    // Owned native presentation for isolated/raster transport; no GPU access.
    bool hasRaster() const;
    QImage raster() const;
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
    std::uint64_t rgbaReadbacks=0,gpuPresentations=0,gpuPresentationAllocations=0;
    // Submitted GL operations, separate from logical copies/presentations.
    std::uint64_t drawCalls=0,drawInstances=0,textureUploadCalls=0,textureUploadBytes=0;
    std::uint64_t programChanges=0,framebufferAttachments=0;
    std::uint64_t paletteTextureUploadCalls=0,paletteTextureUploadBytes=0;
    std::uint64_t paletteCacheHits=0,paletteCacheMisses=0,paletteCacheEvictions=0;
    std::size_t paletteCacheRows=0;
    std::uint64_t presentationPixels=0;
    std::uint64_t scratchAllocations=0,scratchPixels=0,snapshotPixels=0;
    std::uint64_t copyBatches=0,maxCopyBatch=0,nativeSnapshotCopies=0;
    std::uint64_t maskedScratchAllocations=0,maskedScratchPixels=0;
    std::uint64_t recoveryCheckpoints=0,recoveryReadbacks=0,recoveryRebuilds=0;
    std::size_t surfaces=0,pixels=0,coverageProofBytes=0;
};

// Thread-affine rendering owner. Device-recovery mode isolates GL in a service;
// otherwise this owns a dedicated OpenGL 3.3 context. Worker construction
// requires a GUI-created external platform surface that outlives this owner.
// No captured output is
// supplied to draw(): only source pixels, the old destination and the command.
class IsolatedRenderer;
class GlBlitter final {
public:
    explicit GlBlitter(QOpenGLContext* shareContext=nullptr,QOffscreenSurface* externalSurface=nullptr);
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
    // Opt-in diagnostic timestamps in the renderer's own context. At most 64
    // outstanding pairs; a full ring still executes operations and returns
    // false. Poll never waits for query results. Time includes GPU idle gaps
    // during CPU submission; it is a GPU timeline interval, not GPU busy time.
    bool profileBatch(std::uint64_t sequence,const std::function<void()>& operations);
    std::vector<GpuTiming> pollGpuTimings();
    std::size_t pendingGpuTimings() const;
    // Persistent, renderer-owned surfaces: max 2048x2048, 64 handles, 16M pixels.
    // Copies retain textures; read/present are explicit synchronization points.
    SurfaceId create(const Image& image,PixelFormat format);
    // Allocate undefined native storage without a CPU fill/upload. Sampling and
    // presentation remain forbidden until the written region is defined.
    SurfaceId allocate(int width,int height,PixelFormat format);
    // Exact native storage/knowledge/clipper/palette clone; never reads pixels.
    SurfaceId cloneSurface(SurfaceId source);
    void restoreSurface(SurfaceId source,SurfaceId destination);
    // Upload contiguous full-width rows without copying the owned CPU plane.
    void updateRows(SurfaceId surface,const Image& plane,int first,int rows);
    void updateRegionRows(SurfaceId surface,int x,int y,const Image& plane,int first,int rows);
    void destroy(SurfaceId surface);
    void update(SurfaceId surface,int x,int y,const Image& patch);
    // Owned RGB565 source patches with explicit coverage. Uncovered pixels
    // retain GPU history and do not establish validity or require a CPU read.
    void updateMasked(SurfaceId,int x,int y,const Image&,
                      const std::vector<std::uint8_t>& coverage);
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
    // Same-size RGB565 lease import in a sharing context. No CPU pixel return.
    void importGpu565(SurfaceId destination,const GpuFrame& frame);
    void setPalette(SurfaceId surface,unsigned first,const std::vector<Rgb>& colors);
    Image read(SurfaceId surface);
    // Owned metadata only; does not flush, resolve, fence or read pixels.
    QSize dimensions(SurfaceId surface) const;
    QImage present(SurfaceId surface);
    GpuFrame presentGpu(SurfaceId surface);
    RenderStats stats() const;
    // Explicit durable native checkpoint, including partial validity and live
    // frame leases. Requires a quiescent consumer during recovery. These GPU
    // readbacks are recovery work and never diagnostic/original pixel input.
    void checkpointRecovery();
    void recoverContext(QOpenGLContext* shareContext,QOffscreenSurface* externalSurface=nullptr);
    bool hasRecoveryCheckpoint() const;
    // Internal process service entry. The protocol has no host pointers.
    QByteArray dispatchIsolated(const QByteArray&);
    void injectGraphicsFault(unsigned mode);
    static bool isolatedRecoveryEnabled();
private:
    QByteArray saveIsolatedState();
    void loadIsolatedState(const QByteArray&);
    std::unique_ptr<IsolatedRenderer> isolated_;
    std::unordered_map<SurfaceId,std::weak_ptr<GpuFrame::Data>> rasterFrames_;
    std::unordered_map<SurfaceId,GpuFrame> serviceFrames_;
    struct Impl;
    struct Recovery;
    std::shared_ptr<Impl> impl_;
    std::unique_ptr<Recovery> recovery_;
    friend class GpuFrame;
    friend class NativeFrameRecipe;
    ClipperState surfaceClipper(SurfaceId) const;
    void updateCoveredWords(SurfaceId,int,int,const Image&,const std::vector<std::uint8_t>&);
    void copyCoordinates(SurfaceId,std::optional<SurfaceId>,SurfaceId,Rect,int,int,std::optional<std::uint32_t>);
    void assembleMappedWorld(SurfaceId,SurfaceId,SurfaceId,const GpuFrame&);
};
}
