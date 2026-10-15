/* Fixed-build, bounded glyph admission. All trial buffers are privately owned.
 * Other drawing and all layout remain original. No source pointer is cached. */
#include "glyph_live.h"
#include "glyph_route.h"
API u32 WIN GetCurrentThreadId(void);
API HANDLE WIN HeapCreate(u32,u32,u32);
extern void glyph_shadow_leave(void);
static u32 mode,thread,busy,closed,directory_length,capture=1,io_writes;
static char directory[220];
static HANDLE heap;
static u32 stats[36];
static struct Pending {
 u8 *allocation,*frame,*before,*trial;
 u32 canvas,canvas_bytes,frame_address,frame_bytes,stride,width,height,cold,sentinel,format;
 u32 clip[4],incoming[14],predicted[14],header[64],coverage_before[64],coverage_after[64];
 u8 fx_before[512] __attribute__((aligned(16)));
 u8 fx_after[512] __attribute__((aligned(16)));
} pending;
static void copy(void* dst,const void* src,u32 n){u8* d=dst;const volatile u8* s=src;for(u32 i=0;i<n;++i)d[i]=s[i];}
static int same(const void* a,const void* b,u32 n){const volatile u8 *x=a,*y=b;for(u32 i=0;i<n;++i)if(x[i]!=y[i])return 0;return 1;}
static u32 word(u32 at){return *(volatile u32*)at;}
/* Check every region, not just the first VirtualQuery result; reject guards,
 * uncommitted/no-access, wrapping and read-only write destinations. */
static int overlap(u32 a,u32 n,u32 b,u32 m){return a<b+m&&b<a+n;}
static int extent(u32 at,u32 n,int writable){
 if(!at||!n||at>0xffffffffu-n)return 0;
 u32 end=at+n;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,sizeof m)!=sizeof m||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee)||(writable&&!(m[5]&0xcc))||m[0]>at||m[3]>0xffffffffu-m[0])return 0;u32 next=m[0]+m[3];if(next<=at)return 0;at=next<end?next:end;}
 return 1;
}
static HANDLE output(const char* suffix){char name[260];u32 n=0;while(n<directory_length){name[n]=directory[n];++n;}while(*suffix)name[n++]=*suffix++;name[n]=0;return CreateFileA(name,0x40000000,1,0,2,0x80,0);}
static int write(HANDLE f,const void* p,u32 n){u32 done=0;++io_writes;return WriteFile(f,p,n,&done,0)&&done==n;}
static void flush(void){if(!mode)return;HANDLE f=output("\\glyph-stats.bin");if(f==(HANDLE)-1){++stats[11];return;}if(!write(f,stats,sizeof stats))++stats[11];CloseHandle(f);}
void glyph_live_close(void){u32 error=GetLastError();closed=1;flush();SetLastError(error);}
void glyph_live_installed(void){stats[5]=1;flush();}
int glyph_live_active(void){return mode!=0;}
int glyph_live_configure(void){
 char flag[16];u32 n=GetEnvironmentVariableA("MNM_GLYPH_BACKEND",flag,sizeof flag);
 if(!n)return 0;
 if(n==6&&same(flag,"shadow",6))mode=1;
 else if(n==8&&same(flag,"takeover",8))mode=2;
 else return -1;
 n=GetEnvironmentVariableA("MNM_GLYPH_CAPTURE",flag,sizeof flag);
 if(n){if(n!=1 || (flag[0]!='0' && flag[0]!='1'))return -1;capture=flag[0]=='1';}
 if(!capture && mode!=2)return -1;
 n=GetEnvironmentVariableA("MNM_PUBLIC_RESIDENT_WORLD",flag,sizeof flag);
 const int resident=n==1 && flag[0]=='1';
 if(resident && (mode!=2 || capture))return -1;
 const char* conflicts[]={"MNM_CANVAS_PRODUCERS","MNM_CANVAS_STARTUP","MNM_SCENE_WORLD","MNM_WORLD_RASTER_BATCH","MNM_WORD_SPRITES"};
 for(u32 i=0;i<5;++i)if(GetEnvironmentVariableA(conflicts[i],flag,sizeof flag) && !(resident && (i==0 || i==3)))return -1;
 directory_length=GetEnvironmentVariableA("MNM_GLYPH_DIR",directory,sizeof directory);
 if(!directory_length||directory_length>=sizeof directory)return -1;
 heap=HeapCreate(0,0,0);if(!heap)return -1;
 copy(stats,capture?"MNMGLST1":"MNMGLST2",8);stats[2]=capture?1:2;stats[3]=sizeof stats;stats[4]=mode;stats[13]=capture?65536:0;
 flush();return stats[11]?-1:(int)mode;
}
static int globals(void){
 if(!extent(0x658174,4,0)||!extent(0x6a2dc8,4,0)||!extent(0x6e1f7c,16,0)||!extent(0x6f5dac,260,1))return 0;
 const u32 clips[]={0x6e0008,0x6cbb6c,0x6a49b8,0x656618};for(u32 i=0;i<4;++i)if(!extent(clips[i],4,0))return 0;
 pending.format=word(0x6e1f88);
 if(pending.format>1||*(volatile u16*)0x6e1f7c!=(pending.format?0x7c00:0xf800)||*(volatile u16*)0x6e1f7e!=(pending.format?0x3e0:0x07e0)||*(volatile u16*)0x6e1f80!=0x001f)return 0;
 pending.canvas=word(0x658174);pending.stride=word(0x6a2dc8);
 for(u32 i=0;i<4;++i)pending.clip[i]=word(clips[i]);
 pending.width=pending.clip[2];pending.height=pending.clip[3];
 if(!pending.width||!pending.height||pending.width>2048||pending.height>2048||pending.stride<pending.width||pending.stride>2048||pending.clip[0]>=pending.width||pending.clip[1]>=pending.height)return 0;
 pending.canvas_bytes=pending.stride*pending.height*2;pending.sentinel=word(0x6f5eac);
 if(pending.sentinel!=0&&pending.sentinel!=0x3f800000)return 0;
 pending.cold=pending.sentinel==0;return extent(pending.canvas,pending.canvas_bytes,1);
}
static int recheck(void){
 const u32 clips[]={0x6e0008,0x6cbb6c,0x6a49b8,0x656618};
 if(!extent(pending.canvas,pending.canvas_bytes,1)||!extent(pending.frame_address,pending.frame_bytes,0)||!extent(0x6f5dac,260,1))return 0;
 if(word(0x658174)!=pending.canvas||word(0x6a2dc8)!=pending.stride||word(0x6f5eac)!=pending.sentinel||word(0x6e1f88)!=pending.format||*(volatile u16*)0x6e1f7c!=(pending.format?0x7c00:0xf800)||*(volatile u16*)0x6e1f7e!=(pending.format?0x3e0:0x07e0)||*(volatile u16*)0x6e1f80!=0x001f)return 0;
 for(u32 i=0;i<4;++i)if(word(clips[i])!=pending.clip[i])return 0;
 return same(pending.frame,(void*)pending.frame_address,pending.frame_bytes)&&same(pending.before,(void*)pending.canvas,pending.canvas_bytes)&&same(pending.coverage_before,(void*)0x6f5dac,256);
}
static void release(void){if(pending.allocation)HeapFree(heap,0,pending.allocation);pending.allocation=0;__sync_lock_release(&busy);}
static int refuse(u32 reason){++stats[10];++stats[reason];release();if(capture && (stats[6]&63)==0)flush();return 0;}
static void sample(u32* returned,u8* fx){
 /* Bounded retained cohort: eight distinct frame/geometry/tint/control inputs.
  * Reserve six slots for black, one for tinted and one for cropped/skipped
  * geometry. All admitted shadow calls are compared, not just this cohort. */
 static u32 keys[8][8],categories[3];u32 key[8]={pending.frame_address,pending.incoming[5],pending.incoming[10],pending.incoming[11],pending.incoming[12],pending.incoming[13],pending.clip[1],pending.fx_before[0]|(u32)pending.fx_before[1]<<8};
 for(u32 i=0;i<stats[12];++i)if(same(keys[i],key,sizeof key))return;
 if(stats[12]>=8)return;
 i32 x=(i32)pending.incoming[5]-(i32)word((u32)pending.frame+12),y=(i32)pending.incoming[10]-(i32)word((u32)pending.frame+16);
 u32 w=word((u32)pending.frame+4),height=word((u32)pending.frame+8);
 u32 category=(x<(i32)pending.clip[0]||y<(i32)pending.clip[1]||(long long)x+w>(i32)pending.width||(long long)y+height>(i32)pending.height||(!w&&!height))?2:(pending.incoming[11]||pending.incoming[12]||pending.incoming[13])?1:0;
 if(categories[category]>=(category==0?6u:1u))return;
 u32* h=pending.header;for(u32 i=0;i<64;++i)h[i]=0;
 copy(h,pending.format?"MNMGLIV2":"MNMGLIV1",8);h[2]=pending.format?2:1;h[3]=256;h[4]=mode;h[5]=pending.frame_bytes;h[6]=pending.width;h[7]=pending.height;h[8]=pending.stride;h[9]=pending.canvas_bytes;h[10]=pending.frame_address;h[11]=(u32)(pending.incoming[3]+4);for(u32 i=0;i<4;++i)h[12+i]=pending.clip[i];h[16]=pending.cold;h[17]=pending.sentinel;h[18]=stats[6];h[19]=pending.format;
 copy(h+20,pending.incoming,56);copy(h+34,returned,56);
 char suffix[]="\\glyph-0.bin";suffix[7]=(char)('0'+stats[12]);HANDLE f=output(suffix);
 if(f==(HANDLE)-1){++stats[11];return;}
 int ok=write(f,h,256)&&write(f,pending.frame,pending.frame_bytes)&&write(f,pending.before,pending.canvas_bytes)&&write(f,(void*)pending.canvas,pending.canvas_bytes)&&write(f,pending.coverage_before,256)&&write(f,(void*)0x6f5dac,256)&&write(f,pending.fx_before,512)&&write(f,fx,512);CloseHandle(f);
 if(!ok){++stats[11];return;}copy(keys[stats[12]],key,sizeof key);++stats[12];++categories[category];
}
int glyph_live_enter(u32* saved,u8* fx){
 if(!mode)return 0;
 if(__sync_lock_test_and_set(&busy,1)){__sync_fetch_and_add(stats+14,1);__sync_fetch_and_add(stats+10,1);return 0;}
 if(stats[6]!=0xffffffffu)++stats[6];if(closed||(capture && stats[6]>stats[13]))return refuse(20);
 u32 current=GetCurrentThreadId();if(!thread)thread=current;if(thread!=current)return refuse(14);
 if(!globals()||!extent(saved[6],40,0))return refuse(15);
 pending.frame_address=saved[6];pending.frame_bytes=word(saved[6]);
 if(pending.frame_bytes<40||pending.frame_bytes>1048576||!extent(saved[6],pending.frame_bytes,0))return refuse(15);
 if(overlap(pending.frame_address,pending.frame_bytes,pending.canvas,pending.canvas_bytes)||overlap(pending.frame_address,pending.frame_bytes,0x6f5dac,260)||overlap(pending.canvas,pending.canvas_bytes,0x6f5dac,260)||overlap(pending.canvas,pending.canvas_bytes,(u32)saved,56)||overlap(pending.frame_address,pending.frame_bytes,(u32)saved,56))return refuse(16);
 if(!capture){
  /* On the owner thread, no original callback occurs between checked spans and
   * the kernel's complete integer preflight/commit. Source/destination/table
   * are disjoint. The kernel cannot refuse after its first output mutation. */
  MnmGlyphRouteContext context;u8* c=(u8*)&context;for(u32 i=0;i<sizeof context;++i)c[i]=0;
  MnmGlyphBackend* b=&context.backend;b->state.frame=(const u8*)pending.frame_address;b->state.frame_bytes=pending.frame_bytes;
  b->state.coverage_address=0x6f5dac;b->state.left=(i32)pending.clip[0];b->state.top=(i32)pending.clip[1];
  b->state.right=(i32)pending.width;b->state.bottom=(i32)pending.height;b->state.cold=pending.cold;
  b->pixels=(u16*)pending.canvas;b->width=pending.width;b->height=pending.height;b->stride=pending.stride;
  b->coverage=(u32*)0x6f5dac;b->format=pending.format;
  if(!mnm_glyph_route_owned(saved,fx,&context,pending.frame_address))return refuse(17);
  ++stats[7];++stats[8];release();return 1;
 }
 u32 frame_span=(pending.frame_bytes+15)&~15u;
 u32 total=frame_span+2*pending.canvas_bytes+64;
 pending.allocation=HeapAlloc(heap,0,total);if(!pending.allocation)return refuse(19);
 pending.frame=pending.allocation;pending.before=pending.frame+frame_span;pending.trial=pending.before+pending.canvas_bytes;
 copy(pending.frame,(void*)saved[6],pending.frame_bytes);copy(pending.before,(void*)pending.canvas,pending.canvas_bytes);copy(pending.trial,pending.before,pending.canvas_bytes);
 for(u32 i=0;i<64;++i)pending.trial[pending.canvas_bytes+i]=0xa5;
 copy(pending.coverage_before,(void*)0x6f5dac,256);copy(pending.coverage_after,pending.coverage_before,256);copy(pending.fx_before,fx,512);copy(pending.fx_after,fx,512);copy(pending.incoming,saved,56);
 MnmGlyphRouteContext context;u8* c=(u8*)&context;for(u32 i=0;i<sizeof context;++i)c[i]=0;
 MnmGlyphBackend* b=&context.backend;b->state.frame=pending.frame;b->state.frame_bytes=pending.frame_bytes;b->state.coverage_address=0x6f5dac;b->state.left=(i32)pending.clip[0];b->state.top=(i32)pending.clip[1];b->state.right=(i32)pending.width;b->state.bottom=(i32)pending.height;b->state.cold=pending.cold;b->pixels=(u16*)pending.trial;b->width=pending.width;b->height=pending.height;b->stride=pending.stride;b->coverage=pending.coverage_after;b->format=pending.format;
 int handled=mnm_glyph_route_owned(saved,pending.fx_after,&context,pending.frame_address);
 copy(pending.predicted,saved,56);copy(saved,pending.incoming,56);
 if(!handled)return refuse(17);
 for(u32 i=0;i<64;++i)if(pending.trial[pending.canvas_bytes+i]!=0xa5){++stats[11];return refuse(17);}
 if(!recheck())return refuse(18);
 ++stats[7];u32 cw=pending.fx_before[0]|(u32)pending.fx_before[1]<<8;u32 pc=(cw>>8)&3,rc=(cw>>10)&3;++stats[24+(pc==0?0:pc==2?4:8)+rc];
 if(mode==1){saved[9]=(u32)glyph_shadow_leave;return 0;}
 copy((void*)pending.canvas,pending.trial,pending.canvas_bytes);copy((void*)0x6f5dac,pending.coverage_after,256);copy(saved,pending.predicted,56);copy(fx,pending.fx_after,512);++stats[8];if(capture)sample(saved,fx);release();if(capture && (stats[6]&63)==0)flush();return 1;
}
static int fp_equal(const u8* a,const u8* b){
 if(!same(a,b,5)||!same(a+24,b+24,8)||!same(a+160,b+160,128))return 0;
 u32 top=((a[2]|(u32)a[3]<<8)>>11)&7;
 for(u32 i=0;i<8;++i)if(a[4]&(1u<<((top+i)&7)))if(!same(a+32+i*16,b+32+i*16,10))return 0;
 return 1;
}
void glyph_live_leave(u32* saved,u8* fx){
 u32 error=GetLastError();u32 returned[14];copy(returned,pending.predicted,56);
 /* Current save base is original entry-52. Normalize saved ESP and capture
  * original mutated arguments above the return slot, never trampling them. */
 for(u32 i=0;i<8;++i)if(i!=3)returned[i]=saved[i];returned[8]=saved[8];for(u32 i=0;i<4;++i)returned[10+i]=saved[14+i];
 int ok=extent(pending.frame_address,pending.frame_bytes,0)&&same(pending.frame,(void*)pending.frame_address,pending.frame_bytes)&&extent(pending.canvas,pending.canvas_bytes,0)&&same(pending.trial,(void*)pending.canvas,pending.canvas_bytes)&&same(pending.coverage_after,(void*)0x6f5dac,256)&&fp_equal(pending.fx_after,fx);
 for(u32 i=0;i<8;++i)if(i!=3&&returned[i]!=pending.predicted[i])ok=0;
 if((returned[8]&0xcd5)!=(pending.predicted[8]&0xcd5)||!same(returned+10,pending.predicted+10,16))ok=0;
 if(!ok){++stats[11];flush();ExitProcess(96);}
 ++stats[9];sample(returned,fx);saved[13]=pending.incoming[9];release();if((stats[6]&63)==0)flush();SetLastError(error);
}

#ifdef MNM_SCENE_SELFTEST
/* Controlled native admission only. No original executable is mapped or run.
 * The test host loads this DLL before setting glyph environment variables. */
__declspec(dllexport) u32 WIN GlyphProductionForTest(void){
 const u32 addresses[5]={0x650000,0x6a0000,0x6c0000,0x6e0000,0x6f0000};
 const u8* image=GetModuleHandleA(0);if((u32)image!=0x400000)return 1;
 const u32 pe=*(const u32*)(image+0x3c),image_bytes=*(const u32*)(image+pe+24+56);
 if(image_bytes<0x300000)return 1;
 for(u32 i=0;i<5;++i)if(!extent(addresses[i],65536,1))return 1;
 u16* pixels=VirtualAlloc(0,4096,0x3000,4);u8* frame=VirtualAlloc(0,4096,0x3000,4);if(!pixels || !frame)return 2;
 for(u32 i=0;i<64;++i)pixels[i]=(u16)(i*7919+113);
 *(u32*)frame=52;*(u32*)(frame+4)=2;*(u32*)(frame+8)=1;*(u32*)(frame+40)=48;*(u32*)(frame+44)=50;
 frame[48]=0;frame[49]=2;frame[50]=0;frame[51]=63;
 *(u32*)0x658174=(u32)pixels;*(u32*)0x6a2dc8=8;*(u32*)0x6e0008=0;*(u32*)0x6cbb6c=0;
 *(u32*)0x6a49b8=8;*(u32*)0x656618=8;*(u16*)0x6e1f80=31;
 if(glyph_live_configure()!=2 || capture || stats[13])return 4;glyph_live_installed();
 const u32 before=io_writes;
 u8 fx[512] __attribute__((aligned(16))),host[512] __attribute__((aligned(16)));
 __asm__ volatile("fxsave %0; fninit; fxsave %1; fxrstor %0":"=m"(host),"=m"(fx)::"memory");
 for(u32 i=0;i<65545;++i){const u32 format=i&1;
  *(u32*)0x6e1f88=format;*(u16*)0x6e1f7c=format?0x7c00:0xf800;*(u16*)0x6e1f7e=format?0x3e0:0x7e0;
  u32 saved[14];for(u32 j=0;j<14;++j)saved[j]=0;
  saved[6]=(u32)frame;saved[5]=2;saved[8]=0x202;saved[10]=2;saved[11]=173;saved[12]=91;saved[13]=237;
  if(!glyph_live_enter(saved,fx)){__asm__ volatile("fxrstor %0"::"m"(host):"memory");return 8;}
  if(saved[6]!=(format?10u:11u) || saved[11]!=21 || saved[12]!=(format?11u:22u) || saved[13]!=29){
   __asm__ volatile("fxrstor %0"::"m"(host):"memory");return 16;}
 }
 __asm__ volatile("fxrstor %0"::"m"(host):"memory");
 if(io_writes!=before || stats[8]!=65545 || stats[10] || stats[11])return 32;
 glyph_live_close();return 0;
}
/* Production VirtualQuery admission, including a range spanning two regions. */
__declspec(dllexport) u32 WIN GlyphAdmissionForTest(void){
 u8* p=VirtualAlloc(0,8192,0x3000,4);if(!p)return 1;u32 old,unused,fail=0;
 if(!overlap((u32)p,4096,(u32)p+4095,1)||overlap((u32)p,4096,(u32)p+4096,1))fail|=256;
 if(!extent((u32)p,8192,1)||extent(0xffffffffu,2,0)||extent((u32)p,0,0)||extent(0,1,0))fail|=2;
 if(!VirtualProtect(p+4096,4096,2,&old))return 4;
 if(!extent((u32)p,8192,0)||extent((u32)p,8192,1))fail|=8;
 if(!VirtualProtect(p+4096,4096,0x104,&unused))return 16;
 if(extent((u32)p,8192,0))fail|=32;
 if(!VirtualProtect(p+4096,4096,1,&unused))return 64;
 if(extent((u32)p,8192,0))fail|=128;
 return fail;
}
#endif
