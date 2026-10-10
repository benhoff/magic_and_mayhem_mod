#include "world_rolling_channel.hpp"
#include <QtEndian>
#include <cstring>
#include <limits>
#include <stdexcept>
namespace mnm::legacy {
static_assert(__atomic_always_lock_free(sizeof(std::uint32_t),nullptr),"Rolling cross-process ownership requires lock-free32-bit atomics");
namespace {
std::uint32_t load(const std::uint32_t* p){return __atomic_load_n(p,__ATOMIC_ACQUIRE);}
void store(std::uint32_t* p,std::uint32_t n){__atomic_store_n(p,n,__ATOMIC_RELEASE);}
bool claim(std::uint32_t* p,std::uint32_t from,std::uint32_t to){return __atomic_compare_exchange_n(p,&from,to,false,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE);}
std::uint32_t checksum(const std::uint8_t* p,std::size_t n){std::uint32_t h=2166136261u;for(std::size_t i=0;i<n;++i)h=(h^p[i])*16777619u;return h;}
std::uint64_t sequence(const std::uint32_t* p){return p[1]|std::uint64_t(p[2])<<32;}
}
void WorldRollingChannel::create(const QString& path,std::uint64_t session){
  if(QSysInfo::ByteOrder!=QSysInfo::LittleEndian||!session)throw std::invalid_argument("Rolling channel requires little endian and a nonzero session");
  QFile f(path);if(!f.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!f.resize(MNM_ROLL_SIZE))throw std::runtime_error("Cannot create rolling channel");
  auto* bytes=f.map(0,MNM_ROLL_SIZE);if(!bytes)throw std::runtime_error("Cannot initialize rolling channel");
  std::memset(bytes,0,MNM_ROLL_SIZE);std::memcpy(bytes,MNM_ROLL_MAGIC,8);auto* p=reinterpret_cast<std::uint32_t*>(bytes);
  p[2]=1;p[3]=64;p[4]=std::uint32_t(session);p[5]=std::uint32_t(session>>32);p[6]=2;
  p[7]=MNM_ROLL_INPUT;p[8]=MNM_ROLL_REPLY;p[9]=MNM_ROLL_SLOT;p[10]=MNM_ROLL_SIZE;
  f.unmap(bytes);
}
WorldRollingChannel::WorldRollingChannel(const QString& path,std::uint64_t session,Role role):file_(path),session_(session),role_(role){
  if(QSysInfo::ByteOrder!=QSysInfo::LittleEndian||!session||(role!=Role::producer&&role!=Role::consumer)||!file_.open(QIODevice::ReadWrite)||file_.size()!=MNM_ROLL_SIZE)throw std::invalid_argument("Invalid rolling channel file/session/role");
  mapping_=file_.map(0,MNM_ROLL_SIZE);if(!mapping_)throw std::runtime_error("Cannot map rolling channel");
  try{identity();if(!claim(word(role==Role::producer?44:48),0,1))throw std::invalid_argument("Rolling channel role already claimed; create a fresh session");attached_=true;}
  catch(...){file_.unmap(mapping_);mapping_=nullptr;throw;}
}
WorldRollingChannel::~WorldRollingChannel(){if(mapping_)file_.unmap(mapping_);}
std::uint32_t* WorldRollingChannel::word(std::size_t at) const{return reinterpret_cast<std::uint32_t*>(mapping_+at);}
std::size_t WorldRollingChannel::slot(std::uint64_t seq) const{return MNM_ROLL_HEADER+((seq-1)%2)*MNM_ROLL_SLOT;}
void WorldRollingChannel::identity() const{
  const auto* p=word(0);
  if(std::memcmp(mapping_,MNM_ROLL_MAGIC,8)||p[2]!=1||p[3]!=64||!session_||p[4]!=std::uint32_t(session_)||p[5]!=std::uint32_t(session_>>32)||p[6]!=2||p[7]!=MNM_ROLL_INPUT||p[8]!=MNM_ROLL_REPLY||p[9]!=MNM_ROLL_SLOT||p[10]!=MNM_ROLL_SIZE||load(word(44))>1||load(word(48))>1||(attached_&&load(word(role_==Role::producer?44:48))!=1)||p[13]||p[14]||load(word(60))>1)
    {if(attached_)store(word(60),1);throw std::invalid_argument("Rolling channel identity changed");}
  if(cancelled())throw std::runtime_error("Rolling channel cancelled");
}
bool WorldRollingChannel::cancelled() const{return load(word(60))!=0;}
void WorldRollingChannel::cancel(){store(word(60),1);}
[[noreturn]] void WorldRollingChannel::refuse(const char* message){cancel();throw std::invalid_argument(message);}
bool WorldRollingChannel::publish(const std::vector<std::uint8_t>& bytes){
  identity();if(role_!=Role::producer)refuse("Rolling consumer cannot publish");
  if(bytes.empty()||bytes.size()>MNM_ROLL_INPUT||published_==std::numeric_limits<std::uint64_t>::max())refuse("Rolling input extent or sequence exhausted");
  const auto next=published_+1;const auto at=slot(next);const auto state=load(word(at));
  if(state>MNM_ROLL_COMPLETE)refuse("Unknown rolling slot state");
  if(published_-reclaimed_>=2||state!=MNM_ROLL_FREE)return false;
  if(!claim(word(at),MNM_ROLL_FREE,MNM_ROLL_WRITING))return false;
  auto* p=word(at);std::memset(p+1,0,60);p[1]=std::uint32_t(next);p[2]=std::uint32_t(next>>32);p[3]=bytes.size();p[4]=checksum(bytes.data(),bytes.size());
  std::memcpy(mapping_+at+64,bytes.data(),bytes.size());store(p,MNM_ROLL_READY);published_=next;return true;
}
std::optional<RollingInput> WorldRollingChannel::poll(){
  identity();if(role_!=Role::consumer||leased_)refuse("Rolling input ownership already leased or wrong role");
  if(consumed_==std::numeric_limits<std::uint64_t>::max())refuse("Rolling sequence exhausted");
  const auto next=consumed_+1,at=slot(next);auto* p=word(at);const auto state=load(p);
  if(state>MNM_ROLL_COMPLETE)refuse("Unknown rolling slot state");
  if(state!=MNM_ROLL_READY)return {};
  if(!claim(p,MNM_ROLL_READY,MNM_ROLL_READING))return {};
  if(sequence(p)!=next||!p[3]||p[3]>MNM_ROLL_INPUT||p[5]||p[6]||p[7]||p[8])refuse("Rolling source metadata invalid");
  for(unsigned i=9;i<16;++i)if(p[i])refuse("Rolling source reserve nonzero");
  RollingInput input{next,{mapping_+at+64,mapping_+at+64+p[3]}};
  if(checksum(input.bytes.data(),input.bytes.size())!=p[4])refuse("Rolling source checksum mismatch");
  leased_=next;return input;
}
void WorldRollingChannel::complete(std::uint64_t seq,const render::Image& image){
  identity();if(role_!=Role::consumer||!leased_||seq!=leased_)refuse("Stale or unordered rolling completion");
  const auto at=slot(seq);auto* p=word(at);
  if(load(p)!=MNM_ROLL_READING||sequence(p)!=seq||image.width<=0||image.height<=0||image.width>2048||image.height>2048||image.pixels.size()!=std::size_t(image.width)*image.height||image.pixels.size()>MNM_ROLL_REPLY/2)refuse("Rolling completion extent/ownership invalid");
  for(auto pixel:image.pixels)if(pixel>65535)refuse("Rolling completion is not RGB565");
  auto* bytes=mapping_+at+64+MNM_ROLL_INPUT;
  for(std::size_t i=0;i<image.pixels.size();++i)qToLittleEndian<std::uint16_t>(std::uint16_t(image.pixels[i]),bytes+i*2);
  p[5]=image.width;p[6]=image.height;p[7]=image.pixels.size()*2;p[8]=checksum(bytes,p[7]);
  store(p,MNM_ROLL_COMPLETE);consumed_=seq;leased_=0;
}
std::optional<RollingCompletion> WorldRollingChannel::reap(){
  identity();if(role_!=Role::producer)refuse("Rolling consumer cannot reclaim");
  if(reclaimed_==published_)return {};
  const auto next=reclaimed_+1,at=slot(next);auto* p=word(at);const auto state=load(p);
  if(state>MNM_ROLL_COMPLETE)refuse("Unknown rolling slot state");
  if(state!=MNM_ROLL_COMPLETE)return {};
  if(sequence(p)!=next||!p[5]||!p[6]||p[5]>2048||p[6]>2048||p[7]!=std::uint64_t(p[5])*p[6]*2||p[7]>MNM_ROLL_REPLY)refuse("Rolling reply sequence/extent mismatch");
  for(unsigned i=9;i<16;++i)if(p[i])refuse("Rolling reply reserve nonzero");
  const auto* bytes=mapping_+at+64+MNM_ROLL_INPUT;
  if(checksum(bytes,p[7])!=p[8])refuse("Rolling reply checksum mismatch");
  RollingCompletion reply{next,{int(p[5]),int(p[6]),std::vector<std::uint32_t>(p[7]/2)}};
  for(std::size_t i=0;i<reply.image.pixels.size();++i)reply.image.pixels[i]=qFromLittleEndian<std::uint16_t>(bytes+i*2);
  store(p,MNM_ROLL_FREE);reclaimed_=next;return reply;
}
}
