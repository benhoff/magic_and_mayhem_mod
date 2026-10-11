#include "../runtime/scene/world_deferred.h"
#include <array>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <iostream>
static void check(bool ok){if(!ok)throw std::runtime_error("Deferred ownership/admission mismatch");}
int main(){
 std::vector<unsigned char> storage(MNM_WORLD_DEFERRED_BYTES,0xa5);unsigned used=0,count=0,refusals=0;
 std::array<unsigned,24> header{};header[2]=MNM_PRODUCER_DIB;header[3]=5;header[22]=0x1234;
 std::vector<unsigned char> payload{1,2,3,4};const auto original=header;
 check(mnm_world_defer_append(storage.data(),&used,&count,header.data(),payload.data(),payload.size(),2));
 check(header==original && used==100 && count==1);const auto frozen=storage;
 payload.assign(4,0xff);header[22]=0xffff;check(storage==frozen && storage[96]==1 && storage[99]==4);
 auto reject=[&](const auto& r,unsigned n,unsigned entries,unsigned bytes,const void* data){
  auto scratch=frozen;const auto oldEntries=entries,oldBytes=bytes;
  check(!mnm_world_defer_append(scratch.data(),&bytes,&entries,r.data(),data,n,2));
  check(scratch==frozen && entries==oldEntries && bytes==oldBytes);++refusals;
 };
 auto bad=header;bad[3]=2;reject(bad,4,count,used,payload.data());
 bad=header;bad[2]=MNM_PRODUCER_COPY;bad[4]=2;reject(bad,4,count,used,payload.data());
 bad=header;bad[2]=MNM_PRODUCER_BIND;reject(bad,4,count,used,payload.data());
 bad=header;bad[3]=0;reject(bad,4,count,used,payload.data());
 reject(header,4,MNM_WORLD_DEFERRED_RECORDS,used,payload.data());reject(header,4,count,MNM_WORLD_DEFERRED_BYTES-99,payload.data());
 reject(header,MNM_WORLD_DEFERRED_BYTES,count,used,payload.data());reject(header,4,count,used,nullptr);
 auto copy=header;copy[2]=MNM_PRODUCER_COPY;copy[4]=6;
 check(mnm_world_defer_append(storage.data(),&used,&count,copy.data(),nullptr,0,2));check(count==2 && used==196);
 check(std::equal(frozen.begin(),frozen.begin()+100,storage.begin()));
 // The observed cursor save copies primary1 into scratch5; guarded World2
 // remains independent. Exercise sustained background traffic across the old
 // 64-record ceiling, retaining immutable source ownership and exact bounds.
 auto cursor=copy;cursor[3]=5;cursor[4]=1;unsigned burstUsed=0,burstCount=0;
 for(unsigned i=0;i<MNM_WORLD_DEFERRED_RECORDS;++i)check(mnm_world_defer_append(storage.data(),&burstUsed,&burstCount,cursor.data(),nullptr,0,2));
 check(burstUsed==96*MNM_WORLD_DEFERRED_RECORDS && burstCount==MNM_WORLD_DEFERRED_RECORDS);
 reject(cursor,0,burstCount,burstUsed,nullptr);
 unsigned boundaryUsed=MNM_WORLD_DEFERRED_BYTES-96,boundaryCount=MNM_WORLD_DEFERRED_RECORDS-1;
 check(mnm_world_defer_append(storage.data(),&boundaryUsed,&boundaryCount,copy.data(),nullptr,0,2));
 check(boundaryUsed==MNM_WORLD_DEFERRED_BYTES && boundaryCount==MNM_WORLD_DEFERRED_RECORDS);
 std::cout<<"{\"success\":true,\"refusals\":"<<refusals<<",\"owned_payload_bytes\":4,\"unchanged_prefix_bytes\":100,\"cursor_records\":"<<MNM_WORLD_DEFERRED_RECORDS<<",\"exact_byte_record_limits\":true}\n";
}
