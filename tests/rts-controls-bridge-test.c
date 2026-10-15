/* Synthetic PE32 address space and API/service peers; no original execution. */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "../runtime/rts-controls/bridge.c"
static unsigned checks,error_value,thread_id=1,clock_value,protect_calls,deny_protect;
static unsigned fallback_count,mouse_count,focus_count,spell_count,command_count;
static unsigned last_key[4],last_mouse[11],last_focus;
static unsigned char last_command[21];
#define CHECK(v) do {assert(v);++checks;} while(0)
void* WIN GetModuleHandleA(const char* name){(void)name;return (void*)0x400000;}
u32 WIN GetLastError(void){return error_value;}
void WIN SetLastError(u32 n){error_value=n;}
u32 WIN GetCurrentThreadId(void){return thread_id;}
u32 WIN GetTickCount(void){return clock_value;}
u32 WIN GetEnvironmentVariableA(const char* name,char* value,u32 size){
    CHECK(!strcmp(name,"MNM_RTS_CONTROLS_ACTIVE")&&size>=2);value[0]='1';value[1]=0;return 1;
}
u32 WIN VirtualQuery(const void* p,void* out,u32 size){
    struct Region {void* base;void* allocation;u32 protection,size,state,protect,type;};
    if((u32)p<0x400000||(u32)p>=0x800000||size!=sizeof(struct Region))return 0;
    struct Region value={(void*)0x400000,(void*)0x400000,0x40,0x400000,0x1000,0x40,0};
    memcpy(out,&value,sizeof(value));return sizeof(value);
}
int WIN VirtualProtect(void* p,u32 size,u32 protection,u32* old){
    (void)p;(void)size;(void)protection;*old=0x40;
    return ++protect_calls!=deny_protect;
}
void* WIN VirtualAlloc(void* address,u32 size,u32 allocation,u32 protection){
    (void)address;(void)allocation;(void)protection;CHECK(size==12);return (void*)0x790000;
}
HANDLE WIN GetCurrentProcess(void){return (void*)1;}
int WIN FlushInstructionCache(HANDLE p,const void* at,u32 size){(void)p;(void)at;(void)size;return 1;}
static short WIN polling(int vk){return vk==16||vk==17||vk==18?((short*)0x780000)[vk]:0;}
static u32 THIS fallback(void* p,u32 w,u32 m,u32 vk,u32 lp){
    CHECK(p==(void*)0x6cbb78);last_key[0]=w;last_key[1]=m;last_key[2]=vk;last_key[3]=lp;
    ++fallback_count;SetLastError(0x88);return 777;
}
static u32 THIS mouse_peer(void* p,u32 a,u32 b,u32 c,u32 d,u32 e,u32 f,
                           u32 g,u32 h,u32 shift,u32 control,u32 alt){
    CHECK(p==(void*)0x689930);u32 values[]={a,b,c,d,e,f,g,h,shift,control,alt};
    memcpy(last_mouse,values,sizeof(values));++mouse_count;SetLastError(0x88);return 99;
}
static void THIS select_peer(void* value){u8* p=value;put(p+0x68,1);put((void*)0x6e984c,get((void*)0x6e984c)+1);}
static void THIS deselect_peer(void* value){u8* p=value;put(p+0x68,0);put(p+0x70,0);put((void*)0x6e984c,get((void*)0x6e984c)-1);}
static void THIS focus_peer(void* camera,void* actor){CHECK(camera==(void*)0x689930);++focus_count;last_focus=get(actor);}
static void THIS position_peer(void* camera,u32 x,u32 y,u32 z){
    CHECK(camera==(void*)0x689930);put((u8*)camera+0x39,x/32);put((u8*)camera+0x3d,y/32);
    put((u8*)camera+0x41,z/16);++focus_count;
}
static u32 THIS spell_peer(void* bar,u32 slot){CHECK(bar==(void*)0x6a2dd0);put((u8*)bar+0x1908,slot);++spell_count;return 0;}
static void THIS command_peer(void* manager,const void* command){
    CHECK(manager==(void*)0x6a2898);memcpy(last_command,command,21);++command_count;
}
static void jump_peer(u32 address,void* peer){*(u8*)address=0xe9;put((void*)(address+1),(u32)peer-address-5);}
static u8* actor(unsigned slot){return (u8*)(0x720000+slot*0xe4b);}
static void anchors(void){
    const u32 sites[]={0x46c070,0x4fdad0,0x5073d0,0x5074f0,0x4f7ae0,0x4f7a20,0x4d4d60,0x537bd0,0x5060a0};
    const u8 bytes[][8]={{0x83,0xec,0x14,0x53,0x55,0x56,0x8b,0x74},
      {0x81,0xec,0x30,1,0,0,0x53,0x55},{0x57,0x8b,0xf9,0x8b,0x8f,0xa8,0,0},
      {0x53,0x57,0x8b,0xf9,0x33,0xdb,0x39,0x5f},{0x57,0x8b,0x7c,0x24,8,0x85,0xff,0x74},
      {0x55,0x8b,0x44,0x24,8,0x56,0x8b,0x74},{0x83,0xec,0x14,0x55,0x56,0x8b,0xf1,0x57},
      {0x8b,0x41,0x34,0x85,0xc0,0x75,4,0x33},{0x6a,0xff,0x68,0x1b,0x24,0x5c,0,0}};
    for(unsigned i=0;i<9;++i)memcpy((void*)sites[i],bytes[i],i==8?7:8);
    put((void*)0x5c5dec,0x46c070);put((void*)0x5c6a54,0x4fdad0);
}
static u32 press(u32 vk,u32 mods,int repeat){
    for(unsigned i=0;i<3;++i)((short*)0x780000)[16+i]=(mods&(1u<<i))?(short)0x8000:0;
    error_value=0x77;
    return ((KeyFn)get((void*)0x5c5dec))((void*)0x6cbb78,123,mods&4?0x104:0x100,vk,repeat?0x40000000:0);
}
int main(void){
    CHECK(mmap((void*)0x400000,0x400000,PROT_READ|PROT_WRITE|PROT_EXEC,
               MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==(void*)0x400000);
    anchors();*(u8*)0x4d4d60=0xcc;CHECK(!install());CHECK(get((void*)0x5c5dec)==0x46c070);
    anchors();protect_calls=0;deny_protect=2;CHECK(!install());
    CHECK(get((void*)0x5c5dec)==0x46c070&&get((void*)0x5c6a54)==0x4fdad0);
    anchors();protect_calls=deny_protect=0;CHECK(install()&&enabled);
    CHECK(get((void*)0x5c5dec)==(u32)&key&&get((void*)0x5c6a54)==(u32)&mouse);
    jump_peer(0x46c070,&fallback);jump_peer(0x4fdad0,&mouse_peer);
    jump_peer(0x5073d0,&select_peer);jump_peer(0x5074f0,&deselect_peer);
    jump_peer(0x4f7ae0,&focus_peer);jump_peer(0x4f7a20,&position_peer);
    jump_peer(0x4d4d60,&spell_peer);jump_peer(0x537bd0,&command_peer);
    const u8 reset_tail[]={0x83,0xc4,8,0xb8,0x34,0x12,0,0,0xc2,4,0};memcpy((void*)0x5060a7,reset_tail,sizeof(reset_tail));
    put((void*)0x6f34e0,0x6cbb78);put((void*)0x6cbb7c,2);put((void*)0x689920,5);
    put((void*)0x5c5290,(u32)&polling);put((void*)0x6def58,0x720000);
    put((void*)0x6def5c,6);put((void*)0x6df180,6);
    put((void*)0x689955,0x770000);put((void*)0x770004,16);put((void*)0x770008,16);put((void*)0x77000c,4);
    put((void*)0x6c5494,16);put((void*)0x6c5498,16);
    for(unsigned i=0;i<6;++i){
        u8* p=actor(i);put(p,i);put(p+4,1);put(p+0xe4,100);put(p+0xa8,i?14:0);
        put(p+0x8e9,UINT32_MAX);put(p+0x906,UINT32_MAX);put(p+0x5ec,1);
        u32 list=0x760000+i*64;put(p+0x8f9,list);put((void*)(list+4),list+16);put((void*)(list+16),list+16);
        put(p+0xac,0x761000);
    }
    put(actor(3)+0x174,1);put(actor(4)+0x906,0);put(actor(5)+0xe4,0);
    CHECK(press(112,0,0)==0&&get(actor(0)+0x68)==1&&error_value==0x77);
    clock_value=100;CHECK(press(112,0,1)==0&&focus_count==0);
    CHECK(press(112,0,0)==0&&focus_count==1&&last_focus==0);
    CHECK(press(113,0,0)==0&&get(actor(1)+0x68)==1);
    CHECK(press(113,1,0)==0&&get(actor(1)+0x68)&&get(actor(2)+0x68)&&!get(actor(4)+0x68));
    CHECK(press(49,1,0)==0&&get(actor(1)+0x8e9)==1&&get(actor(2)+0x8e9)==1);
    put(actor(2)+0x68,0);CHECK(press(49,3,0)==0&&get(actor(1)+0x8e9)==UINT32_MAX&&get(actor(2)+0x8e9)==1);
    CHECK(press(49,0,0)==0&&get(actor(2)+0x68));clock_value=200;
    CHECK(press(49,0,0)==0&&last_focus==2);
    CHECK(press(115,0,0)==0&&last_focus==2&&error_value==0x77);
    u32 old_group=get(actor(2)+0x8e9);thread_id=2;
    CHECK(press(50,2,0)==777&&get(actor(2)+0x8e9)==old_group);thread_id=1;
    unsigned old_fallback=fallback_count;put((void*)0x6c4634,1);
    CHECK(press(112,0,0)==777&&fallback_count==old_fallback+1);put((void*)0x6c4634,0);
    CHECK(press(81,0,0)==777&&last_key[0]==123&&last_key[1]==0x100&&last_key[2]==81&&error_value==0x88);
    put((void*)(0x6a2dd0+0x1518),2);put((void*)(0x6a2dd0+0x18ec),(u32)actor(0));
    put((void*)(0x6a2dd0+0x1908),UINT32_MAX);
    CHECK(press(81,4,0)==0&&spell_count==1);
    CHECK(press(81,4,0)==0&&spell_count==1); /* already targeted */
    CHECK(press(87,4,1)==0&&spell_count==1);
    CHECK(press(87,4,0)==0&&spell_count==2);
    put((void*)(0x6a2dd0+0xc6+0x1b5),1);
    CHECK(press(87,4,0)==777&&spell_count==2); /* inventory item is not a spell */
    put((void*)(0x689930+0x39),5);put((void*)(0x689930+0x3d),7);
    CHECK(press(116,2,0)==0&&camera_saved[0]);put((void*)(0x689930+0x39),1);
    CHECK(press(116,4,0)==0&&get((void*)(0x689930+0x39))==5);
    unsigned generation=generations[2];CHECK(((ResetFn)0x5060a0)(actor(2),2)==0x1234);
    CHECK(generations[2]==generation+1);
    ((ResetFn)0x5060a0)(actor(0),0);CHECK(!camera_saved[0]);
    u32 session_before=session;put((void*)0x6c4830,10);CHECK(capture());put((void*)0x6c4830,1);
    CHECK(capture()&&session!=session_before);
    put((void*)(0x6a2898+0x28),1);put((void*)(0x6a2898+0x30),1);put((void*)(0x6a2898+0x34),1);
    put((void*)(0x6a2898+0x24),0x775000);put((void*)0x775000,0x776000);
    put((void*)0x775004,4096);put((void*)0x775008,0x777000);put((void*)0x77500c,0);
    put(actor(2)+8,4);put(actor(2)+12,8);put(actor(2)+16,1);
    CHECK(press(83,0,0)==0&&command_count==1&&get(last_command+4)==5&&get(last_command+8)==21);
    CHECK(last_command[12]==2&&last_command[13]==4&&last_command[15]==8&&last_command[17]==1);
    put((void*)0x775004,0);CHECK(press(83,0,0)==777&&command_count==1);
    MouseFn callback=(MouseFn)get((void*)0x5c6a54);
    CHECK(callback((void*)0x689930,1,2,3,4,5,6,7,8,0,1,0)==99&&last_mouse[8]==1);
    CHECK(callback((void*)0x689930,4,2,3,4,5,6,7,8,0,1,0)==99&&last_mouse[8]==0);
    CHECK(last_mouse[0]==4&&last_mouse[1]==2&&last_mouse[9]==1&&mouse_count==2);
    put((void*)0x689920,3);CHECK(callback((void*)0x689930,1,2,3,4,5,6,7,8,0,1,0)==99&&last_mouse[8]==0);
    printf("{\"success\":true,\"checks\":%u,\"scope\":\"synthetic adapter only\"}\n",checks);
}
