#include "../compat/legacy/world_continuous.hpp"
#include "../protocols/include/mnm/world_continuous_checksum.h"
#include <QTemporaryDir>
#include <cassert>
#include <cstring>
#include <iostream>
using namespace mnm::legacy;
template<class F> void refuses(F f) {
  bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}
  assert(rejected);
}
int main() {
  unsigned char request[64]{};
  for(unsigned i=4;i<16;++i)mnm_ack_put(request,i,100+i);
  std::vector<unsigned char> ack(MNM_CONT_ACK_BYTES);
  mnm_ack_encode(ack.data(),request,1,2,3,4);
  assert(mnm_ack_valid(ack.data(),ack.size(),request,1,2,3,4));
  for(unsigned i=0;i<ack.size();++i) {
    ack[i]^=1;assert(!mnm_ack_valid(ack.data(),ack.size(),request,1,2,3,4));ack[i]^=1;
  }
  assert(!mnm_ack_valid(ack.data(),79,request,1,2,3,4));
  assert(!mnm_ack_valid(ack.data(),81,request,1,2,3,4));
  assert(!mnm_ack_valid(ack.data(),80,request,2,2,3,4));
  assert(!mnm_ack_valid(ack.data(),80,request,1,3,3,4));
  assert(!mnm_ack_valid(ack.data(),80,request,1,2,4,4));
  assert(!mnm_ack_valid(ack.data(),80,request,1,2,3,5));
  assert(!mnm_ack_valid(ack.data(),80,request,0,0,3,4));
  QTemporaryDir dir;assert(dir.isValid());auto path=dir.filePath("resident");
  WorldContinuousChannel::create(path,0x200000003ull,512,true);
  WorldContinuousChannel channel(path);assert(channel.resident());
  QFile file(path);assert(file.open(QIODevice::ReadWrite));auto*m=file.map(0,MNM_CONT_BYTES);assert(m);
  mnm_ack_put(m,9,1);
  for(unsigned g=1;g<=512;++g) {
    auto*s=m+MNM_CONT_HEADER+((g-1)&1)*MNM_CONT_SLOT_BYTES;
    std::memset(s,0,MNM_CONT_SLOT_HEADER);mnm_ack_put(s,2,g);
    mnm_ack_put(s,4,64);mnm_ack_put(s,5,64);
    auto*src=s+MNM_CONT_SLOT_HEADER;auto*req=src+MNM_CONT_SOURCE_BYTES;
    std::memset(src,0,64);std::memcpy(req,request,64);
    mnm_ack_put(s,6,mnm_cont_crc32c(src,64));mnm_ack_put(s,7,mnm_cont_crc32c(req,64));
    mnm_ack_put(s,0,MNM_CONT_REQUEST);WorldContinuousChannel::Block block;
    assert(channel.poll(block)&&block.generation==g);
    refuses([&]{channel.reply(g,std::vector<unsigned char>(MNM_CONT_REPLY_BYTES));});
    mnm_ack_encode(ack.data(),request,g,0,3,2);
    ack[56]^=1;refuses([&]{channel.reply(g,ack);});ack[56]^=1;
    ack[44]^=1;refuses([&]{channel.reply(g,ack);});ack[44]^=1;
    mnm_ack_put(m,5,0);refuses([&]{channel.reply(g,ack);});mnm_ack_put(m,5,1);
    channel.reply(g,ack);
    assert(mnm_ack_word(s,10)==80&&mnm_ack_word(s,11)==mnm_cont_crc32c(ack.data(),80));
    assert(!std::memcmp(req+MNM_CONT_REQUEST_BYTES,ack.data(),80));
    mnm_ack_put(s,8,256);mnm_ack_put(s,0,MNM_CONT_RETURNED);channel.release(g);
  }
  channel.cancel();refuses([&]{channel.reply(513,ack);});assert(file.unmap(m));
  auto legacy=dir.filePath("legacy");WorldContinuousChannel::create(legacy,5,0);
  WorldContinuousChannel old(legacy);assert(!old.resident());
  std::cout<<"512 V4 leases: checked80-byte ACKs; all-byte mutation, stale generation/nonce, request digest, pixel reply, immutable mode and cancellation refused\n";
}
