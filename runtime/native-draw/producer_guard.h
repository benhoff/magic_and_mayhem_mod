#include "../../protocols/include/mnm/native_pixel_guard_v1.h"
static u32 np_no_forward,np_guard_installed[6];
static const u32 np_guard_routes[6]={3,8,1,7,1,23};
static void np_guard_fail(u32 reason,u32 family,u32 route,u32 event){
 u32 r[32],done=0;for(u32 i=0;i<32;++i)((volatile u32*)r)[i]=0;
 r[0]=0x504d4e4d;r[1]=0x31445247;r[2]=MNM_PIXEL_GUARD_VERSION;r[3]=MNM_PIXEL_GUARD_BYTES;
 r[4]=reason;r[5]=family;r[6]=route;r[7]=event;r[8]=np_no_forward;
 for(u32 i=0;i<6;++i){r[9+i]=__atomic_load_n(np_guard_installed+i,__ATOMIC_ACQUIRE);r[15+i]=(1u<<np_guard_routes[i])-1;}
 HANDLE file=CreateFileA(MNM_PIXEL_GUARD_FILE,0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,r,sizeof(r),&done,0);CloseHandle(file);}
 ExitProcess(126);
}
static void np_guard_start(void){
 char flag[4];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_NATIVE_PIXEL_NO_FORWARD",flag,4);
 if(!n){if(GetLastError()!=203)np_guard_fail(1,0,0,0);return;}
 if(n!=1||(flag[0]!='0'&&flag[0]!='1'))np_guard_fail(1,0,0,0);
 np_no_forward=flag[0]=='1';
}
static void np_guard_count(u32 family,u32 route,u32 event){
 if(!np_no_forward)return;
 if(family>=6||route>=np_guard_routes[family]||event>=3)np_guard_fail(3,family,route,event);
 const u32 bit=1u<<route;
 if(!event){if(__atomic_fetch_or(np_guard_installed+family,bit,__ATOMIC_ACQ_REL)&bit)np_guard_fail(4,family,route,event);return;}
 if(event==2)np_guard_fail(6,family,route,event);
 if(!(__atomic_load_n(np_guard_installed+family,__ATOMIC_ACQUIRE)&bit))np_guard_fail(7,family,route,event);
 for(u32 i=0;i<6;++i)if(__atomic_load_n(np_guard_installed+i,__ATOMIC_ACQUIRE)!=(1u<<np_guard_routes[i])-1)np_guard_fail(5,family,route,event);
}
