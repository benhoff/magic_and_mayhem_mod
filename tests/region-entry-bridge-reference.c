#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "region_bridge_bytes.h"
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void clear(void* p,u32 n){for(u32 i=0;i<n;++i)((u8*)p)[i]=0;}
static void host(u32 id,u32 action,u32 generation,u32 argument){
 u32* h=menu_words+4;u32 seq=menu_load(h);menu_store(h,seq+1);menu_store(h+3,id);menu_store(h+4,action);menu_store(h+5,generation);menu_store(h+6,argument);menu_store(h,seq+2);
}
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
 copy((void*)0x4b3420,action_bytes,sizeof(action_bytes));copy((void*)0x4ce730,select_bytes,sizeof(select_bytes));
 const u8 transition[]={0xc6,0x41,0x0c,1,0xc7,0x41,0x0d,0,0,0,0,0xc3};copy((void*)0x557510,transition,sizeof(transition));
 u8* p=(u8*)0x6578c0;clear(p,0x100);clear((void*)0x659408,0xa40);clear((void*)0x6e3000,0x1000);
 put(p,0x5c667c);put(p+4,18);put(p+8,1);put(p+0x7c,4);put(p+0x6f,1);put(p+0x73,0x6e3900);put(p+0x78,0x6e3980);copy((void*)0x6e3900,"Celtic",7);copy((void*)0x6e3980,"Forest of Pain",15);
 put(p+0x5b,0x6e3800);put((void*)0x6e3800,0x5c66ac);put((void*)0x6e3804,0x4b3420);put((void*)0x6e3808,(u32)p);
 put(p+0x53,0x6e3100);put(p+0x5f,0x6e3000);put(p+0x67,4);put(p+0x6b,4);put(p+0x4b,0x6e3600);
 for(u32 i=0;i<4;++i){u8* r=(u8*)(0x6e3100+i*0x7e);put((void*)(0x6e3000+i*4),(u32)r);put(r,0x5c6d40);put(r+8,1);put(r+0x2d,i);put(r+0x39,i==0);}
 for(u32 i=0;i<2;++i){u8* b=(u8*)(0x6e3600+i*0x59);put(b,0x5c6c6c);put(b+8,1);put(b+0x2d,i);put(b+0x25,0x6e3800);}
 put((void*)0x659408,0x5c6a60);put((void*)0x65940c,4);put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);put((void*)0x689920,5);
 put((void*)0x6f349c,4);put((void*)0x6f34a8,0x657ce0);put((void*)0x6f34ac,0x659408);put((void*)0x6f34b0,(u32)p);put((void*)0x6f34e0,(u32)p);
 menu_init();require(menu_words&&(menu_version==7||menu_version==8)&&!menu_retired,2);u8 snapshot[MNM_MENU_V7_REGION_SIZE];require(region_snapshot(p,snapshot,menu_version>=8),3);
 // Each invalid layout must reject before touching original radio selection.
 const u32 fields[]={0,4,8,0x7c,0x6f,0x67,0x6b,0x63,0x5b,0x53,0x5f,0x4b};
 for(u32 i=0;i<sizeof(fields)/4;++i){u32 before=get(p+fields[i]);put(p+fields[i],0xffffffff);require(!region_snapshot(p,snapshot,menu_version>=8),10+i);put(p+fields[i],before);}
 p[0x77]=1;require(!region_snapshot(p,snapshot,menu_version>=8),30);p[0x77]=0;put((void*)0x659410,1);require(!region_snapshot(p,snapshot,menu_version>=8),31);put((void*)0x659410,0);
 put((void*)0x6e3004,0x6e3100);require(!region_snapshot(p,snapshot,menu_version>=8),32);put((void*)0x6e3004,0x6e317e);
 SetLastError(0xabc123);menu_poll(p,0);require(menu_ready&&menu_screen==18&&GetLastError()==0xabc123,33);
 u32 request=0;
 for(u32 choice=0;choice<4;++choice){host(++request,MNM_MENU_REGION_DIFFICULTY,menu_generation,choice);menu_poll(p,1);
  require(menu_request==request&&menu_status==MNM_MENU_OK&&get(p+0x63)==choice&&GetLastError()==0xabc123,40+choice);
  for(u32 i=0;i<4;++i)require(get((void*)(0x6e3139+i*0x7e))==(i==choice),44);
 }
 host(++request,MNM_MENU_REGION_DIFFICULTY,menu_generation,4);menu_poll(p,1);require(menu_status==MNM_MENU_INVALID&&get(p+0x63)==3,45);
 host(++request,MNM_MENU_REGION_CANCEL,menu_generation-1,0);menu_poll(p,1);require(menu_status==MNM_MENU_STALE&&!get(p+0x43),46);
 p[0xc]=1;menu_poll(p,0);host(++request,MNM_MENU_REGION_CANCEL,menu_generation,0);menu_poll(p,1);require(menu_status==MNM_MENU_UNAVAILABLE&&!get(p+0x43),47);p[0xc]=0;
 put((void*)0x6e317e+0x3d,2);menu_poll(p,0);host(++request,MNM_MENU_REGION_DIFFICULTY,menu_generation,1);menu_poll(p,1);require(menu_status==MNM_MENU_UNAVAILABLE&&get(p+0x63)==3,48);put((void*)(0x6e317e+0x3d),0);
 put((void*)0x689924,2);put((void*)0x659e3f,0);*(u8*)0x659e50=1;menu_poll(p,0);host(++request,MNM_MENU_REGION_CANCEL,menu_generation,0);menu_poll(p,1);
 require(menu_status==MNM_MENU_OK&&get(p+0x43)==1&&get((void*)0x659e3f)==1&&!*(u8*)0x659e50&&get((void*)0x689924)==2&&GetLastError()==0xabc123,49);
 put((void*)0x659e3f,0);menu_poll(p,1);require(!get((void*)0x659e3f),50); // Consumed ID never repeats.
 if(menu_version>=8){
  put(p+0x43,0);p[0xc]=0;put((void*)0x659f51,0);put((void*)0x659f55,9);put((void*)0x65ae1d,1);put((void*)0x65ae21,1);put((void*)0x65b1dd,1);
  copy((void*)0x54eec0,admission_bytes,sizeof(admission_bytes));put((void*)0x659e33,0);put((void*)0x689924,2);
  put((void*)0x6e363d,2);menu_poll(p,0);host(++request,MNM_MENU_REGION_ENTER,menu_generation,0);menu_poll(p,1);
  require(menu_status==MNM_MENU_UNAVAILABLE&&!get(p+0x43)&&get((void*)0x689924)==2,51);put((void*)0x6e363d,0);
  put((void*)0x65ae21,2);menu_poll(p,0);host(++request,MNM_MENU_REGION_ENTER,menu_generation,0);menu_poll(p,1);require(menu_status==MNM_MENU_UNAVAILABLE&&!get(p+0x43),52);put((void*)0x65ae21,1);
  menu_poll(p,0);host(++request,MNM_MENU_REGION_ENTER,menu_generation,0);menu_poll(p,1);
  require(menu_status==MNM_MENU_OK&&menu_handoff==3&&get((void*)0x689924)==3&&get(p+0x43)==1&&get((void*)0x659e33)==4&&*(u8*)0x659e50==1,53);
  require(get((void*)0x6f2d0c)==1&&get((void*)0x6f2d10)==0&&get((void*)0x6f2d14)==1&&!get((void*)0x659e3f)&&GetLastError()==0xabc123,54);
  put((void*)0x689924,2);menu_poll(p,1);require(get((void*)0x689924)==2,55);
 }
 ExitProcess(0);
}
