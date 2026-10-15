#include "../../protocols/include/mnm/drawing_input_point_v1.h"
/* Owned observation cache. IDs are threads and journal positions, not pointers.
 * Caller holds journal_lock during cache update. */
static u32 input_point_enabled,input_point_overflow;
static u32 input_point_threads[16][7];
static int input_point_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_INPUT_POINT",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1')return 0;input_point_enabled=1;return 1;
}
static void input_point_record(const struct Context* c){
 if(!enabled||!journal||!input_point_enabled||c->r[1]!=2||c->r[2]!=3)return;
 u32* slot=0;
 for(u32 i=0;i<16;++i)if(input_point_threads[i][0]==c->r[6]){slot=input_point_threads[i];break;}
 if(!slot)for(u32 i=0;i<16;++i)if(!input_point_threads[i][0]){slot=input_point_threads[i];break;}
 if(!slot){input_point_overflow=1;return;}
 slot[0]=c->r[6];slot[1]=c->r[0];slot[2]=c->r[29];slot[3]=c->r[11];slot[4]=c->r[31]?c->r[27]:0;slot[5]=c->r[31]?c->r[28]:0;slot[6]=c->r[31]?1:0;
}
static int input_point_writable(void* pointer,u32 bytes){
 u32 at=(u32)pointer,end=at+bytes;if(!at||end<at)return 0;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||m[5]&0x101)return 0;
  u32 p=m[5]&0xff;if(p!=4&&p!=8&&p!=0x40&&p!=0x80)return 0;u32 next=m[0]+m[3];if(next<=at)return 0;at=next;
 }return 1;
}
u32 WIN DrawingInputPointV1(u32 owner,void* output,u32 size){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);u32 result=0;
 if(owner&&size==MNM_DRAWING_INPUT_POINT_V1_BYTES&&input_point_writable(output,size)){
  EnterCriticalSection(journal_lock);
  if(input_point_enabled&&!input_point_overflow){
   u32 r[16]={1,64,0x40209ca7,enabled&&journal?1u:0u,owner,records,entries,planes,drops,gaps,pending,0,0,0,0,0};
   for(u32 i=0;i<16;++i)if(input_point_threads[i][0]==owner){u32* s=input_point_threads[i];if(s[6])r[3]|=2;r[11]=s[1];r[12]=s[2];r[13]=s[3];r[14]=s[4];r[15]=s[5];break;}
   copy(output,r,64);result=1;
  }
  LeaveCriticalSection(journal_lock);
 }
 restore_fp(fp);SetLastError(error);return result;
}
