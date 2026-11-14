/* Original secondary queue fragment and pixel bodies in a private PE mapping.
 * Only the private loop exit and, for the native half, two raster entries change.
 * No installed executable or game process is modified. */
#include "../runtime/shadow/win32_min.h"
#include "../reconstruction/rendering/raster_pixels.h"
#include "../runtime/scene/world_raster_callers.h"
#include "kind8-sprite-original.inc"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
extern void secondary_black_call(void*,int,int,void*,u32*);
void* memset(void* p,int v,u32 n){volatile u8* b=p;while(n--)*b++=(u8)v;return p;}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void require(int yes,u32 code){if(!yes)ExitProcess(code);}
static u8 frame[128],saved_frame[128],node[64];
static u16 original[304],native[304];
static u32 palettes[2][3+255*128],scripted,black_value,clipped_value,black_calls,clipped_calls,guarded;
static u32 callbacks[2];
static volatile u8 reserve[0x380000];
static int expected_x,expected_y;
static u32 __attribute__((noinline)) draw(u32 tag,const u32* stack,void* source,int x,int y,int shade,u32 ordinal){
 const u32 pc=tag==12?0x500a16:0x500a32;
 require(stack[0]==pc&&mnm_world_raster_caller(pc,tag,(const u8*)pc),30);++guarded;++callbacks[tag==17];
 require(source==frame&&x==expected_x&&y==expected_y,31);
 if(tag==17)require(shade==-127&&ordinal==1,32);
 if(scripted){if(tag==12){++black_calls;return black_value;}++clipped_calls;return clipped_value;}
 const u32 w=get(frame+4),h=get(frame+8);if(!w||!h)return 0xa5a50000u;
 const int left=x-(int)get(frame+12),top=y-(int)get(frame+16),cl=(int)get((void*)0x6e0008),cr=(int)get((void*)0x6a49b8);
 if(tag==12&&(left<cl||(long long)left+w>=cr))return 0xa5a50001u;
 struct MnmPixelPlane p={(u16*)get((void*)0x658174),16,12,20,0};struct MnmRasterPixels r={0};
 r.frame=frame;r.bytes=get(frame);r.indexed=1;r.palette_step=2;r.mode=tag==12?7:0;r.contiguous=tag!=12;r.period=16;
 r.left=left;r.top=top;r.clip_left=cl;r.clip_right=cr;r.clip_top=(int)get((void*)0x6cbb6c);r.clip_bottom=(int)get((void*)0x656618);
 r.palette=(const u8*)(palettes[1]+3);require(mnm_raster_pixels(&p,&r),33);return 0xa5a50000u;
}
static u32 __attribute__((noinline)) black(void* f,int x,int y){return draw(12,(u32*)__builtin_frame_address(0)+1,f,x,y,0,0);}
static u32 __attribute__((noinline)) clipped(void* f,int x,int y,int s,u32 o){return draw(17,(u32*)__builtin_frame_address(0)+1,f,x,y,s,o);}
static void bind(u32 address,void* target){*(u8*)address=0xe9;put((void*)(address+1),(u32)target-address-5);}
static void setup(u16* pixels,const int* clip){
 put((void*)0x658174,(u32)(pixels+32));put((void*)0x6a2dc8,20);put((void*)0x6e1f68,0);put((void*)0x6e1f88,0);put((void*)0x6de6e1,0);
 put((void*)0x6e0008,clip[0]);put((void*)0x6cbb6c,clip[1]);put((void*)0x6a49b8,clip[2]);put((void*)0x656618,clip[3]);
}
static void probe(int x,int y,u32* result){
 memset(node,0x5a,sizeof(node));secondary_black_call(frame,x,y,node,result);
 require(result[1]==(u32)y&&result[2]==(u32)node&&result[3]==(u32)frame&&result[4]==(u32)x&&!result[5],34);
 require((get(node+30)&65535)==(result[0]&65535),35);
 for(u32 i=0;i<sizeof(node);++i)if(i!=30&&i!=31)require(node[i]==0x5a,36);
}
static void load_original(void){
 reserve[sizeof(reserve)-1]=1;u32 protection;
 require((u32)GetModuleHandleA(0)==0x400000&&(u32)palettes+sizeof(palettes)<0x480000&&VirtualProtect((void*)0x480000,0x2a0000,0x40,&protection),6);
 char path[512];require(GetEnvironmentVariableA("MNM_SECONDARY_ORIGINAL",path,sizeof(path))<sizeof(path),1);
 HANDLE f=CreateFileA(path,0x80000000,1,0,3,0,0);require(f!=(HANDLE)-1,2);u32 high=0,n=GetFileSize(f,&high),read=0;
 require(!high&&n>1024&&n<8*1024*1024,3);u8* raw=HeapAlloc(GetProcessHeap(),0,n);require(raw&&ReadFile(f,raw,n,&read,0)&&read==n&&CloseHandle(f),4);
 const u32 pe=get(raw+60),opt=pe+24;require(pe+24<n&&get(raw+pe)==0x4550&&get(raw+opt+28)==0x400000,5);
 const u32 length=get(raw+opt+56);require(length==0x302000,6);
 const u32 count=raw[pe+6]|(u32)raw[pe+7]<<8,optional=raw[pe+20]|(u32)raw[pe+21]<<8;
 for(u32 i=0;i<count;++i){const u8* row=raw+opt+optional+i*40;require(row+40<=raw+n,7);const u32 size=get(row+16),off=get(row+20),rva=get(row+12);require(off<=n&&size<=n-off&&rva<=length&&size<=length-rva,8);const u32 begin=rva<0x80000?0x80000:rva;if(begin<rva+size)copy((void*)(0x400000+begin),raw+off+begin-rva,rva+size-begin);}
 require(HeapFree(GetProcessHeap(),0,raw),9);
 require(equal((void*)0x500a0e,secondary_black_branch,sizeof(secondary_black_branch))&&equal((void*)0x5009d2,secondary_black_store,sizeof(secondary_black_store)),10);
 *(u8*)0x500d05=0xc3; /* Exit before queue traversal, preserving both stores. */
}
void start(void){
 load_original();u8 black_body[6],clipped_body[6];copy(black_body,(void*)0x595677,6);copy(clipped_body,(void*)0x57de00,6);
 put(palettes[0],(u32)palettes[1]);palettes[0][2]=palettes[1][2]=127;
 for(u32 bank=0;bank<2;++bank)for(u32 t=1;t<255;++t)for(u32 i=0;i<256;++i)((u16*)(palettes[bank]+3))[t*256+i]=(u16)(t*257+i*97+bank*13);
 bind(0x595677,black);bind(0x57de00,clipped);scripted=1;expected_x=-123;expected_y=456;u32 ax_cases=0;
 for(u32 mode=0;mode<2;++mode)for(u32 hi=0;hi<3;++hi)for(u32 low=0;low<65536;++low){
  const u32 high=hi==0?0:hi==1?0x12340000u:0xffff0000u;black_value=high|(mode?1:low);clipped_value=high|(mode?low:(low*31337+7)&65535);
  black_calls=clipped_calls=0;u32 q[6];probe(expected_x,expected_y,q);const int fallback=(black_value&65535)==1;
  require(q[0]==(fallback?clipped_value:0)&&black_calls==1&&clipped_calls==(u32)fallback,37);++ax_cases;
 }
 scripted=0;const int xs[]={-9,-2,0,2,5,9,13,16,22},ys[]={-6,-1,0,4,9,12,19},origins[]={-3,0,2};const int clips[][4]={{0,0,16,12},{2,1,14,10}};u32 pixel_cases=0,words=0;
 for(u32 shape=0;shape<3;++shape)for(u32 variation=0;variation<2;++variation)for(u32 ox=0;ox<3;++ox)for(u32 oy=0;oy<3;++oy)for(u32 clip=0;clip<2;++clip)for(u32 ix=0;ix<9;++ix)for(u32 iy=0;iy<7;++iy){
  memset(frame,0,sizeof(frame));put(frame,108);put(frame+4,shape==1?0:7);put(frame+8,shape==2?0:4);put(frame+12,origins[ox]);put(frame+16,origins[oy]);put(frame+28,(u32)palettes);
  for(u32 y=0;y<4;++y){put(frame+40+y*8,72+y*4);put(frame+44+y*8,88+y*5);const u8 control[]={1,3,1,2};copy(frame+72+y*4,control,4);for(u32 i=0;i<5;++i)frame[88+y*5+i]=(u8)((y*5+i)*7+variation);}
  copy(saved_frame,frame,sizeof(frame));for(u32 i=0;i<304;++i)original[i]=native[i]=(u16)(i*39+0x1234);
  copy((void*)0x595677,black_body,6);copy((void*)0x57de00,clipped_body,6);setup(original,clips[clip]);u32 a[6],b[6];probe(xs[ix],ys[iy],a);
  bind(0x595677,black);bind(0x57de00,clipped);setup(native,clips[clip]);expected_x=xs[ix];expected_y=ys[iy];probe(xs[ix],ys[iy],b);
  require(equal(original,native,sizeof(original))&&(a[0]&65535)==(b[0]&65535)&&equal(frame,saved_frame,sizeof(frame)),38);words+=304;++pixel_cases;
 }
 copy((void*)0x595677,black_body,6);copy((void*)0x57de00,clipped_body,6);
 require(equal((void*)0x500a0e,secondary_black_branch,sizeof(secondary_black_branch))&&equal((void*)0x5009d2,secondary_black_store,sizeof(secondary_black_store)),39);
 require(ax_cases==393216&&pixel_cases==6804&&callbacks[0]&&callbacks[1],40);
 char output[512];u32 size=GetEnvironmentVariableA("MNM_SECONDARY_PROOF",output,sizeof(output));require(size&&size<sizeof(output),41);
 HANDLE f=CreateFileA(output,0x40000000,0,0,1,0,0);u32 proof[]={0x53424c32,ax_cases,pixel_cases,words,guarded,callbacks[0],callbacks[1],1,1,1},written=0;
 require(f!=(HANDLE)-1&&WriteFile(f,proof,sizeof(proof),&written,0)&&written==sizeof(proof)&&CloseHandle(f),42);ExitProcess(0);
}
