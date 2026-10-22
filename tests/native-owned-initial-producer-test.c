/* Exercise the production source-wrapper initialization emitter with scalar
 * facade proof providers. No original pixels or driver allocations are used. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
typedef void* HANDLE;
#define WIN
#define MNM_PRODUCER_FILL 5
struct Surface {u32 object,id,width,height;};
static u32 continuous_public_bind=1,continuous_resident=1;
static u32 query_result=1,module_present=1,symbol_present=1,readable_object=1;
static u32 query_calls,emissions,writes,closed_bytes,last_record[24];
static int readable(u32 object,u32 bytes){return readable_object&&object==0x1000&&bytes==12;}
static u32 get(const void* p){return (uintptr_t)p==0x1008?0x2000:0x3000;}
static u32 query(void* object,u32 width,u32 height){
 assert((uintptr_t)object==0x2000||(uintptr_t)object==0x3000);
 assert(width==3&&height==2);++query_calls;return query_result;
}
static HANDLE GetModuleHandleA(const char* name){assert(!strcmp(name,"MnmRender.dll"));return module_present?(HANDLE)1:0;}
static void* GetProcAddress(HANDLE module,const char* name){assert(module==(HANDLE)1);assert(!strcmp(name,"RenderOwnedInitialZeroV1"));return symbol_present?(void*)query:0;}
static void extent(u32* r,const struct Surface* s){r[3]=s->id;r[5]=s->width;r[6]=s->height;}
static void Sleep(u32 n){assert(n==0);}
static void emit(const u32* r,const void* payload,u32 bytes){assert(!payload&&!bytes);memcpy(last_record,r,sizeof(last_record));++emissions;}
static void path(char* out,const char* prefix,u32 n,const char* suffix){assert(!strcmp(prefix,"owned-initial-history-")&&!n&&!strcmp(suffix,".bin"));strcpy(out,"owned.bin");}
static HANDLE CreateFileA(const char* name,u32 access,u32 share,void* security,u32 creation,u32 flags,HANDLE template){assert(!strcmp(name,"owned.bin")&&access==0x40000000&&share==3&&!security&&creation==1&&flags==0x80&&!template);return (HANDLE)1;}
static void write(HANDLE file,const void* data,u32 bytes){assert(file==(HANDLE)1&&data);++writes;closed_bytes+=bytes;}
static void CloseHandle(HANDLE file){assert(file==(HANDLE)1);}
#include "../runtime/scene/owned_initial_history.h"
int main(void){
 struct Surface s={0x1000,7,3,2};
 for(unsigned mode=0;mode<7;++mode){
  continuous_public_bind=mode!=0;continuous_resident=mode!=1;readable_object=mode!=2;
  module_present=mode!=3;symbol_present=mode!=4;query_result=mode!=5;s.object=mode==6?0:0x1000;
  producer_initial_history(&s);assert(!emissions&&!owned_initial_count);
 }
 continuous_public_bind=continuous_resident=readable_object=module_present=symbol_present=query_result=1;s.object=0x1000;
 producer_initial_history_close();assert(!writes);query_calls=0;
 for(u32 i=0;i<512;++i){s.id=i+1;producer_initial_history(&s);assert(emissions==i+1&&owned_initial_count==i+1);
  assert(last_record[2]==5&&last_record[3]==s.id&&last_record[5]==3&&last_record[6]==2);
  assert(last_record[12]==3&&last_record[13]==2&&last_record[22]==0x4e495a31);
  for(unsigned k=0;k<24;++k)if(k!=2&&k!=3&&k!=5&&k!=6&&k!=12&&k!=13&&k!=22)assert(!last_record[k]);
  assert(owned_initial_rows[i][0]==s.id&&owned_initial_rows[i][1]==3&&owned_initial_rows[i][2]==2&&owned_initial_rows[i][3]==0x4e495a31);
 }
 producer_initial_history(&s);assert(emissions==512&&query_calls==512);
 producer_initial_history_close();producer_initial_history_close();assert(writes==2&&closed_bytes==16+512*16);
 puts("{\"success\":true,\"preflight_refusals\":7,\"exact_capacity\":512,\"one_over_refused\":true,\"idempotent_close\":true}");
}
