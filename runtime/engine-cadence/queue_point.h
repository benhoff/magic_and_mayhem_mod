#include "../../protocols/include/mnm/engine_cadence_queue_point_v1.h"
/* Same-owner calls cannot interleave a cadence_record while copying this point. */
u32 WIN EngineCadenceQueuePointV1(void* queue,u32 caller,u32* out,u32 size){
 u32 error=GetLastError();if(!enabled||!queue||!out||size!=64||GetCurrentThreadId()!=owner||!count||(!frequency[0]&&!frequency[1])){SetLastError(error);return 0;}
 u32 at=(u32)out,end=at+64;if(end<at){SetLastError(error);return 0;}while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||!(m[5]&(4|8|0x40|0x80))||(m[5]&0x100)||m[0]+m[3]<=at){SetLastError(error);return 0;}at=m[0]+m[3];}
 const u32* r=rows[count-1];u32 caller_id=caller==0x4fd4b0?1:caller==0x4fd559?2:0;
 if(!caller_id||r[1]!=8||r[2]!=owner||r[12]!=(u32)queue||r[13]!=caller||r[5]!=get(0x6c4830)||r[6]!=get(0x6cbb78+0xfd62)||r[10]!=*(u8*)(0x6cbb78+0xfd5a)){SetLastError(error);return 0;}
 u32 p[16]={64,1,r[0],r[5],r[6],r[10],owner,caller_id,r[3],r[4],frequency[0],frequency[1],8,get(0x6f34e0)==0x6cbb78,0,0};for(u32 i=0;i<16;++i)out[i]=p[i];SetLastError(error);return 1;
}
