#include "canvas_world.hpp"
#include "../../protocols/include/mnm/canvas_producers_v1.h"
#include <cstring>
#include <stdexcept>
namespace mnm::legacy {
namespace {
int signedWord(unsigned w){std::int32_t v;std::memcpy(&v,&w,4);return v;}
unsigned word(const std::vector<std::uint8_t>& b,std::size_t at){
  if(at>b.size()||b.size()-at<4)throw std::invalid_argument("Truncated World producer source");
  return b[at]|unsigned(b[at+1])<<8|unsigned(b[at+2])<<16|unsigned(b[at+3])<<24;
}
}
WorldDraw producerWorldDraw(const CanvasProducer &c){
  const auto &r=c.fields;
  if(r[2]!=MNM_PRODUCER_RASTER||r[14]>5||r[15]>10||r[17]>1||
     r[19]<40||r[19]>1048576||c.payload.size()!=r[19]+r[20]||
     word(c.payload,0)!=r[19]||word(c.payload,28)||
     r[20]!=(r[14]==3?64u:r[17]?512u:0u))
    throw std::invalid_argument("Unclosed native World producer request");
  WorldDraw draw;
  draw.frame={bool(r[17]),QByteArray(reinterpret_cast<const char*>(c.payload.data()),r[19])};
  frameIdentity(draw.frame.encoded,draw.frame.indexed);
  draw.x=signedWord(r[8]);draw.y=signedWord(r[9]);
  draw.clip={signedWord(r[10]),signedWord(r[11]),signedWord(r[12]),signedWord(r[13])};
  draw.backend=r[15];draw.composite.mode=render::CompositeMode(r[14]);
  if(r[14]==3){
    draw.composite.rowPeriod=r[16];
    if(!r[16]||r[16]>16)throw std::invalid_argument("Native World displacement period");
    for(unsigned i=0;i<16;++i){draw.composite.rowOffsets[i]=signedWord(word(c.payload,r[19]+i*4));
      if(draw.composite.rowOffsets[i]<0||draw.composite.rowOffsets[i]>16)
        throw std::invalid_argument("Native World displacement offset");}
  }else if(r[17]){
    render::SpriteColourTable colours{};
    for(unsigned i=0;i<256;++i)colours[i]=std::uint16_t(c.payload[r[19]+i*2]|unsigned(c.payload[r[19]+i*2+1])<<8);
    draw.colours=colours;
  }
  return draw;
}
CanvasWorld::CanvasWorld(render::GlBlitter &renderer,const assets::AssetStore &store)
  :renderer_(renderer),store_(store),resources_(store_),bindings_(store_,resources_){}
void CanvasWorld::begin(const CanvasProducer &c,const render::Image &native){
  const auto &r=c.fields;
  if(active_||r[2]!=MNM_PRODUCER_QUEUE_ENTRY||r[14]!=completed_+1||!r[3]||
     (canvas_&&canvas_!=r[3])||native.width!=int(r[5])||native.height!=int(r[6]))
    throw std::invalid_argument("Native producer/World handoff identity or sequence gap");
  if(!scene_)scene_=std::make_unique<render::SceneRenderer>(renderer_,resources_,native);
  scene_->adoptNativeCanvas(native);
  pending_={r[14],r[5],r[6],r[5],{}};
  queue_=r[14];canvas_=r[3];queueDraws_=0;checked_=false;active_=true;
}
void CanvasWorld::append(const CanvasProducer &c){
  if(!active_||c.fields[3]!=canvas_||c.fields[5]!=pending_.width||c.fields[6]!=pending_.height||queueDraws_>=12320)
    throw std::invalid_argument("World producer changed destination or exceeded queue budget");
  auto draw=producerWorldDraw(c);++queueDraws_;++totalDraws_;checked_=false;
  const auto &bytes=c.payload;
  const auto width=word(bytes,4),height=word(bytes,8);
  const auto left=std::int64_t(draw.x)-signedWord(word(bytes,12));
  const auto top=std::int64_t(draw.y)-signedWord(word(bytes,16));
  if((draw.backend==0||draw.backend==9)&&(left<draw.clip.left||left+width>=draw.clip.right))return;
  const auto coverageTop=top+(draw.composite.mode==render::CompositeMode::projectedShadow?height/2:0);
  if(!width||!height||left>=draw.clip.right||left+width<=draw.clip.left||coverageTop>=draw.clip.bottom||coverageTop+height<=draw.clip.top)return;
  pending_.draws.push_back(std::move(draw));
}
render::Image CanvasWorld::complete(const render::Image &reference){
  if(!active_)throw std::runtime_error("No native producer/World queue to complete");
  if(!pending_.draws.empty()){
    const auto draws=bindings_.display(pending_);
    scene_->beginFrame(draws,render::SceneStart::retainedCanvas);
    while(!scene_->drawNext(32)){}
    pending_.draws.clear();
  }
  auto native=scene_->read();
  ++readbacks_;
  if(native.width!=reference.width||native.height!=reference.height||native.pixels!=reference.pixels)
    throw std::runtime_error("Native GPU World differs from independent native producer composition");
  checked_=true;return native;
}
void CanvasWorld::end(const CanvasProducer &c){
  if(!active_||!checked_||!queueDraws_||c.fields[2]!=MNM_PRODUCER_QUEUE_RETURN||
     c.fields[14]!=queue_||c.fields[3]!=canvas_||!pending_.draws.empty())
    throw std::invalid_argument("Incomplete native World return");
  completed_=queue_;active_=false;
}
}
