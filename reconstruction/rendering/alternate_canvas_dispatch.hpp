#pragma once
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
namespace mnm::reconstruction {
// Borrowed storage: this contract never allocates, publishes or retires an
// alternate generation. Base and copy source are explicitly independent.
struct DispatchSpan {std::vector<std::uint16_t>* words=nullptr;std::size_t at=0;};
struct DispatchBinding {DispatchSpan span;std::uint32_t stride=0,height=0;};
struct DispatchClip {std::uint32_t left=0,top=0,right=0,bottom=0;};
struct DispatchState {DispatchBinding binding;DispatchClip clip;};
struct AlternateDispatch {
 bool highResolution=false;
 std::uint32_t xPadding=0,yPadding=0,overlays=0;
 DispatchSpan alternate,copySource;
 DispatchBinding ordinary;
 DispatchClip clip;
};
inline void admitDispatchSpan(DispatchSpan span,std::uint32_t width,
                              std::uint32_t height,std::uint32_t stride){
 if(!span.words||!width||!height||width>4096||height>4096||stride<width||stride>8192||
    span.at>span.words->size()||width>span.words->size()-span.at||
    std::uint64_t(stride)*(height-1)>span.words->size()-span.at-width)
  throw std::invalid_argument("Owned dispatcher span");
}
// 4fd438..4fd5af, with stable borrowed inputs and nonthrowing external callbacks.
// Native admission is explicit policy. Original Lock failures retain original
// ordering: alternate World drawing precedes Lock; ordinary World requires it.
// World/overlay callbacks own their pixels; no original pixel input is needed.
// Returns whether the ordinary Lock succeeded. Unlock's result is ignored.
// Does not install a live hook, infer base/source aliasing or promise rollback
// of external callback failures/reentrancy.
template<class Lock,class World,class Overlay,class Unlock>
bool dispatchAlternateCanvas(const AlternateDispatch& in,DispatchState& state,
                             Lock lock,World world,Overlay overlay,Unlock unlock){
 if(in.xPadding>2048||in.yPadding>2048||in.overlays>256)
  throw std::invalid_argument("Owned dispatcher admission");
 admitDispatchSpan(in.ordinary.span,in.ordinary.stride,in.ordinary.height,in.ordinary.stride);
 const bool alternate=in.alternate.words!=nullptr;
 const std::uint32_t width=(in.highResolution?800:640)+2*in.xPadding,
                     height=(in.highResolution?600:480)+2*in.yPadding;
 if(alternate){
  if(in.ordinary.stride%4||in.clip.right>std::numeric_limits<std::uint32_t>::max()-2*in.xPadding||
     in.clip.bottom>std::numeric_limits<std::uint32_t>::max()-2*in.yPadding)
   throw std::invalid_argument("Owned alternate dimensions");
  admitDispatchSpan(in.alternate,width,height,width);
  admitDispatchSpan(in.copySource,in.ordinary.stride,in.ordinary.height,in.ordinary.stride+2*in.xPadding);
  state.binding={in.alternate,width,height};state.clip=in.clip;
  state.clip.right+=2*in.xPadding;state.clip.bottom+=2*in.yPadding;
  world(state);
 }
 if(!lock())return false;
 state.binding=in.ordinary;
 if(alternate){
  state.clip=in.clip;
  // Real594508 snapshots four WORDs before its stores. Preserve this order
  // when caller-owned source/destination overlap; never replace with memmove.
  for(std::uint32_t y=0;y<in.ordinary.height;++y)
   for(std::uint32_t x=0;x<in.ordinary.stride;x+=4){
    std::array<std::uint16_t,4> chunk;
    const auto from=in.copySource.at+std::size_t(y)*(in.ordinary.stride+2*in.xPadding)+x;
    const auto to=in.ordinary.span.at+std::size_t(y)*in.ordinary.stride+x;
    for(unsigned i=0;i<4;++i)chunk[i]=(*in.copySource.words)[from+i];
    for(unsigned i=0;i<4;++i)(*in.ordinary.span.words)[to+i]=chunk[i];
   }
 }else{world(state);state.clip=in.clip;}
 for(std::uint32_t i=0;i<in.overlays;++i)overlay(state,i);
 unlock();return true;
}
} // namespace mnm::reconstruction
