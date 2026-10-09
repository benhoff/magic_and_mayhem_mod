#include "../compat/legacy/world_raster_batch.hpp"
#include "../protocols/include/mnm/world_raster_batch_v3.h"
#include <cassert>
#include <cstring>
#include <iostream>
using namespace mnm::legacy;
static void put(std::vector<std::uint8_t>& b,unsigned at,unsigned w){for(unsigned k=0;k<4;++k)b.at(at+k)=std::uint8_t(w>>(k*8));}
int main(){
 CanvasProducer black{};auto& r=black.fields;r[0]=140;r[1]=2;r[2]=9;r[3]=7;r[5]=800;r[6]=600;r[8]=799;r[12]=800;r[13]=600;r[18]=44;r[19]=44;r[21]=1;black.payload.resize(44);put(black.payload,0,44);put(black.payload,4,1);put(black.payload,8,1);
 CanvasProducer fallback=black;fallback.fields[1]=3;fallback.fields[15]=3;fallback.fields[21]=0;
 std::vector<const CanvasProducer*> rows{&black,&fallback};std::vector<std::uint8_t> request(96);std::memcpy(request.data(),MNM_WORLD_BATCH_REQUEST,8);
 const unsigned h[]={3,64,1,1,3,7,800,600,800,32,0,1,2,2};for(unsigned i=0;i<14;++i)put(request,8+i*4,h[i]);
 for(unsigned i=0;i<2;++i){put(request,64+i*16,2+i);put(request,68+i*16,i?0x57de00:0x595677);put(request,72+i*16,i?0:1);put(request,76+i*16,worldRasterSourceHash(*rows[i]));}
 auto checksum=[&](auto& b){put(b,48,worldBatchHash({b.begin()+64,b.end()}));};checksum(request);
 auto parse=[&](const auto& b){return validateWorldRasterBatch(b,1,1,3,7,800,600,0,rows);};auto batch=parse(request);assert(batch.descriptors.size()==2);assert(worldRasterAX(black)==1&&worldRasterAX(fallback)==0);
 unsigned refused=0;auto reject=[&](auto b){try{parse(b);}catch(const std::invalid_argument&){++refused;return;}throw std::runtime_error("Malformed batch accepted");};
 for(unsigned at:{0u,8u,12u,16u,20u,24u,28u,32u,36u,40u,44u,48u,52u,56u,60u}){auto b=request;b[at]^=1;reject(b);}
 for(unsigned at:{64u,68u,72u,76u,80u,84u,88u,92u}){auto b=request;b[at]^=1;checksum(b);reject(b);}
 auto truncated=request;truncated.pop_back();reject(truncated);auto extended=request;extended.push_back(0);reject(extended);
 auto alias=request;put(alias,68,0x596490);checksum(alias);reject(alias);
 auto source=request;black.payload[40]^=1;reject(source);black.payload[40]^=1;
 auto reply=worldRasterBatchReply(batch,std::vector<std::uint16_t>(480000,0x1234));assert(reply.size()==960064&&std::memcmp(reply.data(),MNM_WORLD_BATCH_REPLY,8)==0);assert(reply[64]==0x34&&reply[65]==0x12);
 bool badExtent=false;try{worldRasterBatchReply(batch,{0});}catch(const std::invalid_argument&){badExtent=true;}assert(badExtent);
 assert(refused==27);std::cout<<"Complete queue source/entry/AX admission;27 malformed manifests and wrong completion extent refused\n";
}
