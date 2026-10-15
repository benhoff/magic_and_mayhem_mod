/* Opt-in lease metadata. The fault path never takes recorder locks or reads
 * mutable canvas caches. Address overlap is not backing-store ownership. */
static u32 source_write_trace_enabled,source_write_trace_phase;
static u32 source_write_trace_sequence,source_write_trace_queue;
static void source_write_trace_file(const char* stem,const void* data,u32 bytes){
 char name[260];path(name,stem,source_lease_id,".bin");
 HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(file==(HANDLE)-1||!write(file,data,bytes)||!CloseHandle(file))ExitProcess(99);
}
static void source_write_trace_marker(u32 phase){
 if(!source_write_trace_enabled)return;
 u32 r[8]={0};copy(r,phase==2?"MNMSWA01":"MNMSWD01",8);
 r[2]=1;r[3]=32;r[4]=source_lease_id;r[5]=source_lease_canvas;r[6]=source_lease_owner;r[7]=phase;
 source_write_trace_file(phase==2?"source-write-armed-":"source-write-done-",r,32);
 source_write_trace_phase=phase;
}
/* Caller holds the producer recorder scope, before protection changes. */
static int source_write_trace_prepare(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_SOURCE_WRITE_TRACE",flag,2);
 if(!n){source_write_trace_enabled=source_write_trace_phase=0;return 1;}
 if(n!=1||flag[0]!='1')return 0;
 source_write_trace_enabled=1;source_write_trace_phase=1;
 source_write_trace_sequence=sequence;source_write_trace_queue=queue;
 static u32 rows[MAX_SURFACES][16];u32 count=0,source_count=0,memory[7]={0};
 if(VirtualQuery((void*)source_lease_pixels,memory,28)!=28||memory[4]!=0x1000)return 0;
 for(u32 i=0;i<MAX_SURFACES;++i){
  struct Surface* s=surfaces+i;if(!s->object)continue;
  if(!s->id)return 0;for(u32 j=0;j<count;++j)if(rows[j][2]==s->id)return 0;
  u32* r=rows[count];for(u32 j=0;j<16;++j)r[j]=0;r[0]=++count;r[1]=1;r[2]=s->id;
  if(s->id==source_lease_canvas){r[3]|=1;++source_count;}
  r[6]=s->width;r[7]=s->height;r[8]=s->stride;r[10]=s->dirty;r[11]=s->sampled;
  if(s->pixels){
   r[3]|=128;r[4]=s->pixels>=source_lease_base;
   r[5]=r[4]?s->pixels-source_lease_base:source_lease_base-s->pixels;
   if(s->pixels==source_lease_pixels)r[3]|=2;
   u32 query[7]={0};if(VirtualQuery((void*)s->pixels,query,28)==28){
    r[3]|=64;r[12]=query[4];r[13]=query[5];r[14]=query[6];
    if(query[4]==0x1000&&query[1]==memory[1])r[3]|=16;
   }
   if(s->width&&s->width<=2048&&s->height&&s->height<=2048&&s->stride>=s->width&&s->stride<=4096){
    u32 span=((s->height-1)*s->stride+s->width)*2;
    if(s->pixels<=~0u-span){
     r[3]|=32;r[9]=span;
     if(s->pixels<source_lease_pixels+960000&&s->pixels+span>source_lease_pixels)r[3]|=4;
     if(s->pixels<source_lease_base+source_lease_extent&&s->pixels+span>source_lease_base)r[3]|=8;
    }
   }
  }
 }
 if(source_count!=1)return 0;
 u32 h[16]={0};copy(h,"MNMSWT01",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;
 h[5]=source_lease_id;h[6]=source_lease_canvas;h[7]=source_lease_owner;
 h[8]=source_write_trace_queue;h[9]=source_write_trace_sequence;
 h[10]=source_lease_pixels-source_lease_base;h[11]=960000;h[12]=source_lease_extent;h[13]=count;h[14]=128;
 char name[260];path(name,"source-write-trace-",source_lease_id,".bin");
 HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(file==(HANDLE)-1||!write(file,h,64)||!write(file,rows,count*64)||!CloseHandle(file))ExitProcess(99);
 return 1;
}
static void source_write_trace_fault(const struct BatchExceptionRecord* e){
 if(!source_write_trace_enabled)return;
 u32 r[32]={0};copy(r,"MNMSWF01",8);r[2]=1;r[3]=128;
 r[4]=source_lease_id;r[5]=source_lease_canvas;r[6]=GetCurrentThreadId();r[7]=source_lease_owner;
 r[8]=e->information[1]-source_lease_base;
 r[9]=e->information[1]>=source_lease_pixels&&e->information[1]-source_lease_pixels<960000;
 r[10]=source_write_trace_phase;r[11]=e->code;r[12]=e->information[0];
 u32 target[7]={0};if(VirtualQuery((void*)e->information[1],target,28)==28)r[13]=target[5];
 r[14]=source_lease_extent;r[15]=source_lease_pixels-source_lease_base;
 u32 code[7]={0};if(VirtualQuery((void*)e->address,code,28)==28){
  r[16]|=1;r[22]=code[6];
  if(code[4]==0x1000&&code[6]==0x1000000&&code[1]<=e->address&&readable(code[1],64)&&*(u16*)code[1]==0x5a4d){
   u32 offset=get((void*)(code[1]+60));
   if(offset<=0x100000&&code[1]<=~0u-offset-96&&readable(code[1]+offset,96)){
    u32 pe=code[1]+offset;
    if(get((void*)pe)==0x4550&&*(u16*)(pe+4)==0x14c&&*(u16*)(pe+24)==0x10b){
     r[16]|=2;r[17]=e->address-code[1];r[18]=0x14c;r[19]=get((void*)(pe+8));r[20]=get((void*)(pe+80));r[21]=get((void*)(pe+88));
    }
   }
  }
 }
 if(readable(e->address,16)){r[16]|=4;r[23]=16;copy(r+24,(void*)e->address,16);}
 r[28]=source_write_trace_sequence;r[29]=source_write_trace_queue;
 source_write_trace_file("source-write-fault-",r,128);
}
