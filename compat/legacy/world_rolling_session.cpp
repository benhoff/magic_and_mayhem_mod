#include "world_rolling_session.hpp"
#include <chrono>
#include <limits>
namespace mnm::legacy {
WorldRollingSession::WorldRollingSession(render::GlBlitter& renderer,const assets::AssetStore& store)
  :producer_([store](const auto& name){auto opened=store.open(name);if(auto* e=std::get_if<assets::Error>(&opened))throw std::runtime_error(e->detail);auto f=std::get<std::unique_ptr<assets::AssetFile>>(std::move(opened));auto read=assets::readWhole(*f,16*1024*1024);if(auto* e=std::get_if<assets::Error>(&read))throw std::runtime_error(e->detail);return std::get<std::vector<std::uint8_t>>(std::move(read));}),world_(renderer,store){}
render::Image WorldRollingSession::consume(std::uint64_t sequence,const std::vector<std::uint8_t>& bytes,const std::function<void(const CanvasProducer&,const render::Image&)>& checkpoint){
  if(failed_)throw std::runtime_error("Rolling renderer session is terminal; create a fresh session");
  try{
    if(completed_==std::numeric_limits<std::uint64_t>::max()||sequence!=completed_+1)throw std::invalid_argument("Rolling renderer sequence gap or exhaustion");
    if(!cached_||cachedBytes_!=bytes){
      auto decoded=decodeRollingQueue(bytes);auto owned=bytes;
      cached_=std::move(decoded);cachedBytes_=std::move(owned);++decodes_;
    }else ++hits_;
    const auto& stream=*cached_;assertionMs_=0;double cpuMs=0;unsigned canvas=0;
    for(const auto& c:stream.operations){const auto& r=c.fields;const auto start=std::chrono::steady_clock::now();producer_.apply(c);cpuMs+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
      if(r[2]==11){canvas=r[3];world_.beginRolling(c,producer_.read(canvas),sequence);}
      else if(world_.active()){
        if(r[2]==9)world_.append(c);
        else if(r[2]==10&&r[3]==canvas)producer_.commitNativeWorld(canvas,world_.complete(producer_.read(canvas)));
        else if(r[2]==12)world_.end(c);
      }
      if(r[2]==10&&checkpoint){const auto assertStart=std::chrono::steady_clock::now();checkpoint(c,producer_.read(r[3]));assertionMs_+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-assertStart).count();}
    }
    if(world_.active()||world_.completedSequence()!=sequence)throw std::runtime_error("Rolling renderer did not complete the packet");
    const auto& p=world_.profile();nativeMs_=cpuMs+p.adoptMs+p.prepareMs+p.submitMs+p.readbackMs+p.compareMs;
    auto result=producer_.read(canvas);completed_=sequence;return result;
  }catch(...){failed_=true;throw;}
}
WorldRollingSession::PacketCacheStats WorldRollingSession::packetCacheStats() const{
  return {decodes_,hits_,cachedBytes_.capacity(),cached_?cached_->operations.size():0,cached_?cached_->ownedPayloads.size():0,cached_?cached_->ownedPayloadBytes:0};
}
}
