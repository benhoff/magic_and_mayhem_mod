/* Actual owned facade/bridge and PE32 protected stores. Host independently
 * applies the emitted World/masked updates and supplies bound CPU_READ replies. */
#define start retained_resource_probe
#include "native-draw-resource-probe.c"
#undef start
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
API i32 WIN GetObjectA(void*,i32,void*);
API void* WIN GetCurrentObject(void*,u32);
typedef i32 (WIN *History)(const void*,u32,u32,u32,u32,u32);
typedef i32 (WIN *LeaseTransfer)(void*,u32*,const u16*,u32);
typedef i32 (WIN *LeaseSnapshot)(void*,const u32*,u16*,u32);
static void *surface;static volatile u16* pixels;static u32 pitch;
static void* held_dc;
static LeaseTransfer transfer;static LeaseSnapshot snapshot;
static volatile u32 watching,stop_watch,samples;
static u32 WIN foreign_query(void* unused){
 (void)unused;u32 identity[12];require(transfer(surface,identity,0,0)<0,401);
 u32 layout[4]={(u32)pixels,800,600,pitch};u16 untouched=0xbeef;
 require(snapshot(surface,layout,&untouched,800*600*2)<0 && untouched==0xbeef,454);return 0;
}
static u32 WIN foreign_dc_release(void* unused){(void)unused;require(((i32(WIN*)(void*,void*))table(surface)[26])(surface,held_dc)<0,455);return 0;}
static u32 WIN watch(void* unused){
 (void)unused;watching=1;
 while(!stop_watch){u32 m[7];require(VirtualQuery((void*)pixels,m,28)==28 && m[5]==1,402);++samples;}
 return 0;
}
static u32 WIN foreign_read(void* unused){(void)unused;volatile u16 got=pixels[0];(void)got;ExitProcess(403);return 0;}
static void joined(HANDLE h){require(h && !WaitForSingleObject(h,5000),404);CloseHandle(h);}
static void save(const char* name,const void* bytes,u32 size){
 u32 written;HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 require(file!=(HANDLE)-1 && WriteFile(file,bytes,size,&written,0) && written==size,405);CloseHandle(file);
}
void start(void){
 char mode[2];require(GetEnvironmentVariableA("MNM_COHERENCE_CASE",mode,2)==1,406);
 void* native=LoadLibraryA("MnmDraw.dll");require(native!=0,407);
 Create create=(Create)GetProcAddress(native,"NativeDrawCreate");
 History history=(History)GetProcAddress(native,"NativeDrawCpuHistoryV1");
 transfer=(LeaseTransfer)GetProcAddress(native,"NativeDrawCpuLeaseTransferV1");
 snapshot=(LeaseSnapshot)GetProcAddress(native,"NativeDrawCpuLeaseSnapshotV1");
 i32 (WIN *world)(const void*,u32)=(i32(WIN*)(const void*,u32))GetProcAddress(native,"NativeDrawCpuGuardWorldV2");
 require(create && history && transfer && snapshot && world,408);
 void* render=mode[0]=='i'?LoadLibraryA("MnmRender.dll"):0;
 void (WIN *install)(void*,u32)=render?(void(WIN*)(void*,u32))GetProcAddress(render,"RenderInstallForTest"):0;
 u32 (WIN *bind)(u32,u32,u32,u32,u32,u32,u32,u32)=render?(u32(WIN*)(u32,u32,u32,u32,u32,u32,u32,u32))GetProcAddress(render,"RenderResidentWorldV1"):0;
 u32 (WIN *shutdown)(u32)=render?(u32(WIN*)(u32))GetProcAddress(render,"RenderShutdown"):0;
 u32 (WIN *stop)(u32)=render?(u32(WIN*)(u32))GetProcAddress(render,"RenderStop"):0;
 void* draw=0;require(!create(0,&draw,0),409);if(mode[0]=='i'){require(install && bind && shutdown && stop,410);install(draw,1);}
 u32 d[31];desc(d,2);d[2]=600;d[3]=800;
 if(mode[0]=='i'){require(!((i32(WIN*)(void*,void*,u32))table(draw)[20])(draw,0,8),453);d[1]=1;d[26]=0x200;}
 require(!((Surface)table(draw)[6])(draw,d,&surface,0),411);
 if(mode[0]=='d'){
  desc(d,2);require(!((Lock)table(surface)[25])(surface,0,d,1,0),412);pixels=(u16*)d[9];pixels[17]=0xabcd;
  require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),413);
  void* dc=0;require(!((i32(WIN*)(void*,void**))table(surface)[17])(surface,&dc),414);
  held_dc=dc;joined(CreateThread(0,0,foreign_dc_release,0,0,0));
  u32 bm[6];void* bitmap=GetCurrentObject(dc,7);require(GetObjectA(bitmap,24,bm)==24 && bm[5],415);
  require(((volatile u16*)bm[5])[17]==0xabcd && history((void*)pixels,1,1,0,17,34)<0 && !history((void*)pixels,2,0,0,0,0),416);
  require(((volatile u16*)bm[5])[17]==0xabcd,417);
  require(!((i32(WIN*)(void*,void*))table(surface)[26])(surface,dc),418);
 }else if(mode[0]=='i'){
  /* Typed reader tags cover every selected CPU-history family. Each World
   * changes all GPU words, then a pending CPU store must survive restoration. */
  const u32 tags[][2]={{2,1},{2,4},{2,5},{2,0},{5,13},{5,14},{5,16},{5,17},{5,18},{5,19},{5,20},{1,0},{1,1},{1,2},{3,0},{4,0}};
  for(u32 generation=1;generation<=sizeof(tags)/sizeof(tags[0]);++generation){
   desc(d,2);require(!((Lock)table(surface)[25])(surface,0,d,1,0),419);pixels=(u16*)d[9];pitch=d[4];
   require(bind((u32)pixels,800,600,800,generation,0,17,34)==1,420);
   void* rejected_dc=0;require(((i32(WIN*)(void*,void**))table(surface)[17])(surface,&rejected_dc)<0 && !rejected_dc,456);
   pixels[3]=0; /* Equal to the old CPU baseline; explicit dirty tracking matters. */
   /* Ordinary instructions resume outside exception dispatch through the same
    * actual ordered bridge/FIFO transfer as an explicit typed reader. */
   if(generation==1)require(pixels[1]==(u16)(71+generation*19),460);
   if(generation==2)__asm__ volatile("lock addw $0,(%0)"::"r"(pixels+10):"memory","cc");
   if(generation==3){u8 observed[7];const u8* input=(const u8*)pixels;u8* output=observed;u32 count=7;
    __asm__ volatile("rep movsb":"+S"(input),"+D"(output),"+c"(count)::"memory");
    require(observed[0]==(u8)(generation*19)&&observed[6]==0,461);}
   if(generation==4){u32 observed[4];__asm__ volatile("movdqu (%0),%%xmm0;movdqu %%xmm0,(%1)"::"r"(pixels),"r"(observed):"memory");
    require((observed[0]&0xffff)==generation*19,462);}
   SetLastError(0x1234);require(!history((void*)pixels,0,tags[generation-1][0],tags[generation-1][1],0,0) && GetLastError()==0x1234,421);
   for(u32 i=0;i<800*600;++i)require(pixels[i]==(i==3?0:(u16)(i*71+generation*19)),422);
   pixels[7]^=0x1234;
   require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),423);
   if(generation==1){
    /* Previously Unlock incorrectly kept gpu_history after the CPU restore,
     * selecting a GPU-only plan which refused this current source. */
    const i32 one[4]={0,0,1,1};
    require(!((i32(WIN*)(void*,u32,u32,void*,const i32*,u32))table(surface)[7])(surface,0,0,surface,one,0x10),463);
   }
  }
  desc(d,2);require(!((Lock)table(surface)[25])(surface,0,d,1,0),439);pixels=(u16*)d[9];
  require(bind((u32)pixels,800,600,800,17,0,17,34)==1,440);pixels[3]=0;
  require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),441);
  i32 rect[4]={10,0,20,2};desc(d,2);require(!((Lock)table(surface)[25])(surface,rect,d,1,0),442);
  volatile u16* part=(u16*)d[9];require(part==pixels+10,443);part[1]=0xbeef;
  require(!history((void*)part,0,2,5,0,0),444);
  for(u32 i=0;i<800*600;++i)require(pixels[i]==(i==3?0:i==11?0xbeef:(u16)(i*71+17*19)),445);
  part[2]^=0x1234;require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)part),446);
  for(u32 generation=18;generation<=19;++generation){
   desc(d,2);require(!((Lock)table(surface)[25])(surface,0,d,1,0),447);pixels=(u16*)d[9];
   require(bind((u32)pixels,800,600,800,generation,0,17,34)==1,448);
   ((volatile u8*)pixels)[6]=0;
   if(generation==18){
    require(!history((void*)pixels,0,2,3,0,0),449);
    require(pixels[3]==(u16)((3*71+generation*19)&0xff00),450);pixels[7]^=0x1234;
   }
   require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),451);
   require(pixels[3]==(u16)((3*71+generation*19)&0xff00),452);
  }
  /* Counter persistence must not enter application reader/copy/tracker work. */
  require(CreateFileA("native-cpu-read-counts.bin",0x80000000,7,0,3,0x80,0)==(HANDLE)-1,459);
  require(stop(5000) && shutdown(0),424);
 }else{
  desc(d,2);require(!((Lock)table(surface)[25])(surface,0,d,1,0),425);pixels=(u16*)d[9];pitch=d[4];
  require(!history((void*)pixels,1,1,0,17,34),426);
  u32 identity[12];require(!transfer(surface,identity,0,0),427);
  joined(CreateThread(0,0,foreign_query,0,0,0));
  if(mode[0]=='r'){joined(CreateThread(0,0,foreign_read,0,0,0));ExitProcess(428);}
  if(mode[0]=='w'){
   u32 layout[4]={(u32)pixels,800,600,pitch};u16* copy=HeapAlloc(GetProcessHeap(),0,800*600*2);require(copy!=0,429);
   require(world((void*)pixels,1)==1 && snapshot(surface,layout,copy,800*600*2)<0 && history((void*)pixels,1,2,0,17,34)<0,430);
   require(!world((void*)pixels,0) && !snapshot(surface,layout,copy,800*600*2) && !history((void*)pixels,1,2,0,17,34),431);HeapFree(GetProcessHeap(),0,copy);
  }else{
   HANDLE observer=CreateThread(0,0,watch,0,0,0);require(observer!=0,432);while(!watching)Sleep(0);
   for(u32 i=0;i<20000;++i)pixels[i%800]=(u16)i;
   u16* cursor=(u16*)pixels+102;
   __asm__ volatile("std; stosw; cld":"+D"(cursor):"a"(0xa1b2):"memory","cc");
   require(cursor==(u16*)pixels+101,457);
   __asm__ volatile("movb %%ah, (%0)"::"r"(pixels+103),"a"(0x11223344):"memory");
   u16* captured=HeapAlloc(GetProcessHeap(),0,800*600*2);u32 layout[4]={(u32)pixels,800,600,pitch};
   require(captured && !snapshot(surface,layout,captured,800*600*2) && captured[102]==0xa1b2 && ((u8*)captured)[206]==0x33,458);HeapFree(GetProcessHeap(),0,captured);
   stop_watch=1;joined(observer);require(samples>0,433);
  }
  u16* image=HeapAlloc(GetProcessHeap(),0,800*600*2);require(image!=0,434);
  for(u32 i=0;i<800*600;++i)image[i]=0x4321;
  require(!transfer(surface,identity,0,0),435);
  for(u32 i=0;i<12;++i){identity[i]^=1;require(transfer(surface,identity,image,800*600*2)<0,436);identity[i]^=1;}
  require(!transfer(surface,identity,image,800*600*2) && pixels[0]==0x4321,437);HeapFree(GetProcessHeap(),0,image);
  require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)pixels),438);
 }
 ((u32(WIN*)(void*))table(surface)[2])(surface);((u32(WIN*)(void*))table(draw)[2])(draw);
 u32 result[4]={1,(u32)mode[0],samples,GetCurrentThreadId()};save("active-reader-result.bin",result,sizeof(result));ExitProcess(0);
}
