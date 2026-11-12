#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
typedef uint32_t u32;typedef uint8_t u8;
#include "../protocols/include/mnm/native_copy_callers_v1.h"
#include "../protocols/include/mnm/native_copy_callers_v2.h"
#include "../protocols/include/mnm/copy_caller_handoff_v1.h"
#include "../runtime/render/copy_caller_slots.h"
static u8 memory[4*1024*1024];
static int read_memory(void* context,u32 address,void* out,u32 size){
 (void)context;
 if(address<0x400000 || address-0x400000>sizeof(memory) || size>sizeof(memory)-(address-0x400000))return 0;
 memcpy(out,memory+address-0x400000,size);return 1;
}
static void put(u32 address,u32 value){memcpy(memory+address-0x400000,&value,4);}
int main(void){
 const u32 base=0x400000,extent=sizeof(memory),slot=0x700000,caller=0x480105,target=0x600100,wrapper=0x600200;
 u8 states[7];u32 row[16];unsigned cases=0;
 for(unsigned i=0;i<sizeof(copy_caller_sites)/sizeof(copy_caller_sites[0]);++i){
  const struct CopyCallerSite* site=copy_caller_sites+i;const struct CopyCallerLayout* layout=copy_caller_layouts+site->layout;
  memset(memory,0,sizeof(memory));memset(states,1,sizeof(states));
  memory[site->api-3]=0xff;memory[site->api-2]=site->modrm;memory[site->api-1]=site->method;
  put(slot+layout->distance,caller);put(slot+layout->distance+4,wrapper);put(wrapper+8,target);
  memory[caller-base-5]=0xe8;put(caller-4,base+layout->entry-caller);
  assert(copy_caller_resolve(base,extent,base+site->api,slot,site->method==0x1c,target,read_memory,0,states,row)==1);
  assert(row[3]==layout->entry && row[4]==layout->distance && row[5]==caller && row[7]==wrapper);++cases;
  put(wrapper+8,target+1);assert(copy_caller_resolve(base,extent,base+site->api,slot,site->method==0x1c,target,read_memory,0,states,row)==5);put(wrapper+8,target);
  put(caller-4,1);assert(copy_caller_resolve(base,extent,base+site->api,slot,site->method==0x1c,target,read_memory,0,states,row)==6);
  assert(copy_caller_resolve(base,extent,base+site->api,UINT32_MAX,site->method==0x1c,target,read_memory,0,states,row)==4);
  assert(copy_caller_resolve(base,extent,base+site->api,slot,site->method!=0x1c,target,read_memory,0,states,row)==3);
  memset(states,0,sizeof(states));assert(copy_caller_resolve(base,extent,base+site->api,slot,site->method==0x1c,target,read_memory,0,states,row)==3);
 }
 memset(states,1,sizeof(states));assert(copy_caller_resolve(base,extent,0x410000,slot,0,target,read_memory,0,states,row)==2);
 struct MnmCopyCallers* c=calloc(1,sizeof(*c));assert(c);memset(row,0,sizeof(row));row[9]=1;
 for(u32 i=0;i<MNM_COPY_CALLERS_CAPACITY;++i){row[0]=i+1;mnm_copy_callers_record(c,row);}
 assert(c->header[6]==MNM_COPY_CALLERS_CAPACITY && c->header[10]==MNM_COPY_CALLERS_CAPACITY && !c->header[7]);
 mnm_copy_callers_record(c,row);assert(c->header[7] && c->header[6]==MNM_COPY_CALLERS_CAPACITY);
 memset(c,0,sizeof(*c));row[0]=2;mnm_copy_callers_record(c,row);assert(c->header[9] && !c->header[6]);
 memset(c,0,sizeof(*c));row[0]=1;row[9]=7;mnm_copy_callers_record(c,row);assert(c->header[9] && !c->header[6]);
 memset(c,0,sizeof(*c));c->header[8]=UINT32_MAX;mnm_copy_callers_record(c,row);assert(c->header[7] && c->header[8]==UINT32_MAX);
 free(c);
 struct MnmCopyCallersV2* v2=calloc(1,sizeof(*v2));assert(v2);u32 extended[20]={0};extended[9]=1;extended[17]=2;extended[16]=0x600000;extended[18]=5;extended[19]=0x700000;
 for(u32 i=0;i<MNM_COPY_CALLERS_CAPACITY;++i){extended[0]=i+1;mnm_copy_callers_v2_record(v2,extended);}
 assert(v2->rows[0][16]==0x600000 && v2->rows[MNM_COPY_CALLERS_CAPACITY-1][19]==0x700000);
 mnm_copy_callers_v2_record(v2,extended);assert(v2->header[7]);free(v2);
 u32 handoff[32]={MNM_COPY_CALLER_HANDOFF_MAGIC,1,128,0x58bf40,5,0x10000,0x20000,0x30000,12};
 assert(mnm_copy_caller_handoff_valid(handoff,0x58bf40,5,12));
 const unsigned corrupt[]={0,1,2,3,4,5,6,7,8,9,15,19,31};
 for(unsigned i=0;i<sizeof(corrupt)/sizeof(corrupt[0]);++i){u32 value=handoff[corrupt[i]];handoff[corrupt[i]]=value?0:99;assert(!mnm_copy_caller_handoff_valid(handoff,0x58bf40,5,12));handoff[corrupt[i]]=value;}
 handoff[10]=0x480105;assert(!mnm_copy_caller_handoff_valid(handoff,0x58bf40,5,12));
 handoff[9]=1;for(unsigned i=11;i<=14;++i)handoff[i]=i;assert(mnm_copy_caller_handoff_valid(handoff,0x58bf40,5,12));
 printf("{\"success\":true,\"sites\":%u,\"wrong_frame_code_destination_call_and_capacity_refusals\":true,\"versioned_handoff_refusals\":true}\n",cases);return 0;
}
