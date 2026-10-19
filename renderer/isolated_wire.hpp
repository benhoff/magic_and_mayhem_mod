#pragma once
#include "blit.hpp"
#include "dib.hpp"
#include "surface_copy.hpp"
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <type_traits>
#include <stdexcept>
namespace mnm::render::wire {
// Private versioned process protocol. Only owned values cross this boundary.
constexpr quint32 version=1, maxPacket=192*1024*1024;
enum class Op:quint32 { create,allocate,clone,restore,rows,regionRows,destroy,update,masked,dib,copy,composite,colour,additive,invalidate,clip,surfaceCopy,swap,palette,read,dimensions,present,clipper,covered,coordinates,mapped,driver,stats,checkpoint,load,frame,importFrame,fault };
inline void put(QDataStream&,const Rect&);
inline void get(QDataStream&,Rect&);
inline void put(QDataStream&,const PixelFormat&);
inline void get(QDataStream&,PixelFormat&);
inline void put(QDataStream&,const Rgb&);
inline void get(QDataStream&,Rgb&);
inline void put(QDataStream&,const Image&);
inline void get(QDataStream&,Image&);
inline void put(QDataStream&,const ClipperState&);
inline void get(QDataStream&,ClipperState&);
inline void put(QDataStream&,const SpriteComposite&);
inline void get(QDataStream&,SpriteComposite&);
inline void put(QDataStream&,const DibInput&);
inline void get(QDataStream&,DibInput&);
inline void put(QDataStream&,const SurfaceCopyRequest&);
inline void get(QDataStream&,SurfaceCopyRequest&);
inline void put(QDataStream&,const SurfaceCopyResult&);
inline void get(QDataStream&,SurfaceCopyResult&);
inline void put(QDataStream&,const Driver&);
inline void get(QDataStream&,Driver&);
inline void put(QDataStream&,const RenderStats&);
inline void get(QDataStream&,RenderStats&);
inline void put(QDataStream&,const QSize&);
inline void get(QDataStream&,QSize&);
inline void put(QDataStream&,const QImage&);
inline void get(QDataStream&,QImage&);
template<class T> void put(QDataStream& s,const T& v);
template<class T> void get(QDataStream& s,T& v);
template<class T> void put(QDataStream& s,const T& v){
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    if constexpr(sizeof(T)>4)s<<quint64(v);else s<<quint32(v);
}
template<class T> void get(QDataStream& s,T& v){
    static_assert(std::is_integral_v<T> || std::is_enum_v<T>);
    if constexpr(sizeof(T)>4){quint64 n;s>>n;v=T(n);}else{quint32 n;s>>n;v=T(n);}
}
inline void put(QDataStream& s,const QByteArray& v){s<<v;}
inline void get(QDataStream& s,QByteArray& v){s>>v;}
inline void put(QDataStream& s,const std::string& v){s<<QByteArray::fromStdString(v);}
inline void get(QDataStream& s,std::string& v){QByteArray b;s>>b;v=b.toStdString();}
template<class T,std::size_t N> void put(QDataStream& s,const std::array<T,N>& v){for(auto& n:v)put(s,n);}
template<class T,std::size_t N> void get(QDataStream& s,std::array<T,N>& v){for(auto& n:v)get(s,n);}
inline void put(QDataStream& s,const std::vector<std::uint8_t>& v){put(s,QByteArray(reinterpret_cast<const char*>(v.data()),v.size()));}
inline void get(QDataStream& s,std::vector<std::uint8_t>& v){QByteArray b;get(s,b);if(b.size()>16*1024*1024)throw std::runtime_error("Render byte vector budget");v.assign(b.begin(),b.end());}
template<class T> void put(QDataStream& s,const std::vector<T>& v){put(s,quint32(v.size()));for(auto& n:v)put(s,n);}
template<class T> void get(QDataStream& s,std::vector<T>& v){quint32 n;get(s,n);if(n>16*1024*1024)throw std::runtime_error("Render wire vector budget");v.resize(n);for(auto& x:v)get(s,x);}
template<class T> void put(QDataStream& s,const std::optional<T>& v){put(s,bool(v));if(v)put(s,*v);}
template<class T> void get(QDataStream& s,std::optional<T>& v){bool b;get(s,b);if(b){T x;get(s,x);v=std::move(x);}else v.reset();}
template<class... T> void fields(QDataStream& s,const T&... v){(put(s,v),...);}
template<class... T> void values(QDataStream& s,T&... v){(get(s,v),...);}
inline void put(QDataStream& s,const Rect& v){fields(s,v.left,v.top,v.right,v.bottom);}
inline void get(QDataStream& s,Rect& v){values(s,v.left,v.top,v.right,v.bottom);}
inline void put(QDataStream& s,const PixelFormat& v){fields(s,v.bits,v.masks);}
inline void get(QDataStream& s,PixelFormat& v){values(s,v.bits,v.masks);}
inline void put(QDataStream& s,const Rgb& v){fields(s,v.red,v.green,v.blue);}
inline void get(QDataStream& s,Rgb& v){values(s,v.red,v.green,v.blue);}
inline void put(QDataStream& s,const Image& v){fields(s,v.width,v.height,v.pixels);}
inline void get(QDataStream& s,Image& v){values(s,v.width,v.height,v.pixels);}
inline void put(QDataStream& s,const ClipperState& v){fields(s,v.attached,v.regions);}
inline void get(QDataStream& s,ClipperState& v){values(s,v.attached,v.regions);}
inline void put(QDataStream& s,const SpriteComposite& v){fields(s,v.mode,v.rowOffsets,v.rowPeriod,v.retainOutsideDisplacement);}
inline void get(QDataStream& s,SpriteComposite& v){values(s,v.mode,v.rowOffsets,v.rowPeriod,v.retainOutsideDisplacement);}
inline void put(QDataStream& s,const DibInput& v){fields(s,v.header,v.palette,v.pixels,v.usage);}
inline void get(QDataStream& s,DibInput& v){values(s,v.header,v.palette,v.pixels,v.usage);}
inline void put(QDataStream& s,const SurfaceCopyRequest& v){fields(s,v.api,v.source,v.destination,v.flags,v.sourceBusy,v.destinationBusy);}
inline void get(QDataStream& s,SurfaceCopyRequest& v){values(s,v.api,v.source,v.destination,v.flags,v.sourceBusy,v.destinationBusy);}
inline void put(QDataStream& s,const SurfaceCopyResult& v){fields(s,v.hresult,v.pieces);}
inline void get(QDataStream& s,SurfaceCopyResult& v){values(s,v.hresult,v.pieces);}
inline void put(QDataStream& s,const Driver& v){fields(s,v.vendor,v.renderer,v.version);}
inline void get(QDataStream& s,Driver& v){values(s,v.vendor,v.renderer,v.version);}
inline void put(QDataStream& s,const RenderStats& v){fields(s,v.uploads,v.copies,v.paletteUpdates,v.nativeReadbacks,v.presentations,v.rgbaReadbacks,v.gpuPresentations,v.gpuPresentationAllocations,v.drawCalls,v.drawInstances,v.textureUploadCalls,v.textureUploadBytes,v.programChanges,v.framebufferAttachments,v.paletteTextureUploadCalls,v.paletteTextureUploadBytes,v.paletteCacheHits,v.paletteCacheMisses,v.paletteCacheEvictions,v.paletteCacheRows,v.presentationPixels,v.scratchAllocations,v.scratchPixels,v.snapshotPixels,v.copyBatches,v.maxCopyBatch,v.nativeSnapshotCopies,v.maskedScratchAllocations,v.maskedScratchPixels,v.recoveryCheckpoints,v.recoveryReadbacks,v.recoveryRebuilds,v.surfaces,v.pixels,v.coverageProofBytes);}
inline void get(QDataStream& s,RenderStats& v){values(s,v.uploads,v.copies,v.paletteUpdates,v.nativeReadbacks,v.presentations,v.rgbaReadbacks,v.gpuPresentations,v.gpuPresentationAllocations,v.drawCalls,v.drawInstances,v.textureUploadCalls,v.textureUploadBytes,v.programChanges,v.framebufferAttachments,v.paletteTextureUploadCalls,v.paletteTextureUploadBytes,v.paletteCacheHits,v.paletteCacheMisses,v.paletteCacheEvictions,v.paletteCacheRows,v.presentationPixels,v.scratchAllocations,v.scratchPixels,v.snapshotPixels,v.copyBatches,v.maxCopyBatch,v.nativeSnapshotCopies,v.maskedScratchAllocations,v.maskedScratchPixels,v.recoveryCheckpoints,v.recoveryReadbacks,v.recoveryRebuilds,v.surfaces,v.pixels,v.coverageProofBytes);}
inline void put(QDataStream& s,const QSize& v){fields(s,v.width(),v.height());}
inline void get(QDataStream& s,QSize& v){int w,h;values(s,w,h);v={w,h};}
inline void put(QDataStream& s,const QImage& v){auto i=v.convertToFormat(QImage::Format_RGBA8888);fields(s,i.size(),QByteArray(reinterpret_cast<const char*>(i.constBits()),i.sizeInBytes()));}
inline void get(QDataStream& s,QImage& v){QSize z;QByteArray bytes;values(s,z,bytes);if(z.width()<1||z.height()<1||z.width()>2048||z.height()>2048||bytes.size()!=qint64(z.width())*z.height()*4)throw std::runtime_error("Invalid render wire image");v=QImage(reinterpret_cast<const uchar*>(bytes.constData()),z.width(),z.height(),QImage::Format_RGBA8888).copy();}
inline void check(QDataStream& s){if(s.status()!=QDataStream::Ok || !s.atEnd())throw std::runtime_error("Malformed render process message");}
template<class... T> QByteArray message(const T&... v){QByteArray b;QDataStream s(&b,QIODevice::WriteOnly);s.setVersion(QDataStream::Qt_6_0);fields(s,v...);return b;}
template<class T> T decode(const QByteArray& b){QDataStream s(b);s.setVersion(QDataStream::Qt_6_0);T v;get(s,v);check(s);return v;}
}
