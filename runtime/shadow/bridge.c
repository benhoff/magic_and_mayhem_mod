#include "win32_min.h"
/* Logging-only hook. Integer code, no CRT, no threads, no floating arithmetic. */
extern void shadow_enter(void);
extern void shadow_return(void);
u32 shadow_trampoline;
static u32 image_base, samples, limit=1;
static volatile i32 busy;
static char directory[220]="shadow";
static u32 saved_return, output_vector, budget_pointer, before_count;
static u8* record;
static u32 record_size;
static u8 original[6];
static const u8 expected[6]={0x81,0xec,0x8c,0,0,0};
static void copy(void* to,const void* from,u32 n) {
    u8* d=to; const u8* s=from; while(n--) *d++=*s++;
}
static void zero(void* to,u32 n) { u8* d=to; while(n--) *d++=0; }
static u32 word(const void* p) { const u8* b=p; return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24; }
static void put(void* p,u32 v) { u8* b=p; b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24; }
static int equal(const void* a,const void* b,u32 n) {
    const u8* x=a; const u8* y=b; while(n--) if(*x++!=*y++) return 0; return 1;
}
static int readable(u32 address,u32 length) {
    if(!address || address+length<address) return 0;
    u32 end=address+length;
    while(address<end) {
        u32 m[7];
        if(VirtualQuery((void*)address,m,28)!=28 || m[4]!=0x1000 || (m[5]&0x101)) return 0;
        u32 next=m[0]+m[3];
        if(next<=address) return 0;
        address=next<end?next:end;
    }
    return 1;
}
static int read(u32 address,void* to,u32 length) {
    if(!readable(address,length)) return 0; copy(to,(void*)address,length); return 1;
}
static int global(u32 va,u32* result) { return read(image_base+va-0x400000,result,4); }
static void filename(char* path,const char* name,u32 number) {
    u32 at=0; while(directory[at]) {path[at]=directory[at];++at;}
    path[at++]='\\'; while(*name) path[at++]=*name++;
    for(u32 divisor=1000;divisor;divisor/=10) path[at++]=(char)('0'+number/divisor%10);
    path[at++]='.';path[at++]='b';path[at++]='i';path[at++]='n';path[at]=0;
}
static int write_all(const char* name,u32 number,const void* bytes,u32 length) {
    char path[260]; filename(path,name,number);
    HANDLE file=CreateFileA(path,0x40000000,0,0,1,0x80,0); /* CREATE_NEW */
    if(file==(HANDLE)-1) return 0;
    u32 done=0;
    while(done<length) {
        u32 wrote=0;
        if(!WriteFile(file,(const u8*)bytes+done,length-done,&wrote,0) || !wrote) {CloseHandle(file);return 0;}
        done+=wrote;
    }
    return CloseHandle(file)!=0;
}
static int capture_world(const u8* descriptor,u32 budget) {
    u32 x,y,z,plane,cell_base,terrain_base,boundary,default_scalar,slope;
    if(!global(0x6c5494,&x)||!global(0x6c5498,&y)||!global(0x5e1780,&z)||
       !global(0x6c54a0,&plane)||!global(0x6c54dc,&cell_base)||!global(0x65660c,&terrain_base)||
       !global(0x6c5c80,&boundary)||!global(0x6c007d,&default_scalar)||!global(0x5e15f0,&slope)) return 0;
    if(!x||x>1024||!y||y>1024||!z||z>32||!plane) return 0;
    static u32 rows[1024],layers[32];
    u32 max_row=0,max_layer=0;
    if(!read(image_base+0x2cb942,rows,y*4)||!read(image_base+0x2cb8c2,layers,z*4)) return 0;
    for(u32 i=0;i<y;++i) {if(rows[i]>0x200000) return 0;if(rows[i]>max_row)max_row=rows[i];}
    for(u32 i=0;i<z;++i) {if(layers[i]>0x200000) return 0;if(layers[i]>max_layer)max_layer=layers[i];}
    u32 cell_count=max_row+max_layer+x;
    if(cell_count>0x200000 || !readable(cell_base,cell_count*12)) return 0;
    u32 highest=0;
    for(u32 i=0;i<cell_count;++i) {
        u8* cell=(u8*)cell_base+i*12; u32 index=cell[0]|(u32)cell[1]<<8;
        if(index>highest)highest=index;
    }
    u32 object=word(descriptor+4),type_pointer;
    static u8 object_bytes[0xd07];
    if(!read(object,object_bytes,0xd07)) return 0;
    u32 type_index=word(object_bytes+0xa8); type_pointer=word(object_bytes+0xac);
    if(type_index>1024) return 0;
    u32 generator=image_base+0x2a5f80+type_index*0x5c9;
    u32 lengths[7]={y*4,z*4,cell_count*12,(highest+1)*0x164,0xd07,0x198,0x5c9};
    if((!terrain_base && highest)||!readable(type_pointer,0x198)||!readable(generator,0x5c9)||
       (terrain_base && !readable(terrain_base,lengths[3]))) return 0;
    u32 total=92; for(u32 i=0;i<7;++i) total+=lengths[i];
    if(total>64*1024*1024) return 0;
    u8* blob=HeapAlloc(GetProcessHeap(),0,total);
    if(!blob) return 0;
    copy(blob,"MNMWLD01",8);
    u32 fields[14]={object,x,y,z,plane,boundary,budget,word(descriptor+8),
                   word(descriptor+0x2e),word(descriptor+0x32),word(descriptor+0x36),default_scalar,slope,cell_base};
    copy(blob+8,fields,56);copy(blob+64,lengths,28);
    u32 at=92;
    copy(blob+at,rows,lengths[0]);at+=lengths[0];copy(blob+at,layers,lengths[1]);at+=lengths[1];
    copy(blob+at,(void*)cell_base,lengths[2]);at+=lengths[2];
    if(terrain_base)copy(blob+at,(void*)terrain_base,lengths[3]);else zero(blob+at,lengths[3]);at+=lengths[3];
    copy(blob+at,object_bytes,0xd07);at+=0xd07;
    copy(blob+at,(void*)type_pointer,0x198);at+=0x198;copy(blob+at,(void*)generator,0x5c9);
    int ok=write_all("world-",samples,blob,total);
    HeapFree(GetProcessHeap(),0,blob); return ok;
}
static int vector(u32 address,u32* begin,u32* count) {
    u32 header[4]; if(!read(address,header,16))return 0;
    if(header[2]<header[1]||header[3]<header[2]||(header[2]-header[1])%36)return 0;
    *count=(header[2]-header[1])/36;*begin=header[1];
    return *count<=4096 && (!*count||readable(*begin,*count*36));
}
void shadow_pre(u32* frame) {
    u32 error=GetLastError();
    if(!__sync_bool_compare_and_swap(&busy,0,1)) {SetLastError(error);return;}
    if(samples>=limit) {__sync_lock_release(&busy);SetLastError(error);return;}
    u8 descriptor[70],prior[28]; u32 budget,begin;
    if(!read(frame[11],descriptor,70)||!read(frame[12],prior,28)||!read(frame[13],&budget,4)||
       !budget||(i32)budget<0||word(descriptor)!=0||word(descriptor+0x3a)!=0||!vector(frame[10],&begin,&before_count))goto skip;
    record_size=126+before_count*36;
    record=HeapAlloc(GetProcessHeap(),0,record_size+26*36+before_count*36);
    if(!record)goto skip;
    copy(record,"MNMEXP01",8);put(record+8,frame[6]);put(record+12,budget);
    put(record+20,before_count);copy(record+28,descriptor,70);copy(record+98,prior,28);
    if(before_count)copy(record+126,(void*)begin,before_count*36);
    ++samples;
    if(!capture_world(descriptor,budget)) {HeapFree(GetProcessHeap(),0,record);record=0;goto skip;}
    output_vector=frame[10];budget_pointer=frame[13];saved_return=frame[9];
    frame[9]=(u32)&shadow_return;
    SetLastError(error);return;
skip:
    __sync_lock_release(&busy);SetLastError(error);
}
u32 shadow_post(void) {
    u32 error=GetLastError(),result=saved_return,begin,count,budget;
    if(vector(output_vector,&begin,&count) && count>=before_count && count-before_count<=26 &&
       read(budget_pointer,&budget,4)) {
        put(record+16,budget);put(record+24,count);
        if(count)copy(record+record_size,(void*)begin,count*36);
        write_all("expansion-",samples,record,record_size+count*36);
    }
    HeapFree(GetProcessHeap(),0,record);record=0;__sync_lock_release(&busy);
    SetLastError(error);return result;
}
static int install(u32 base) {
    u32 a,b,c,d;
    __asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));
    if(!(d&(1U<<24)))return 0; /* FXSAVE required; unsupported CPU stays original. */
    u32 site=base+0xeaae0,protection;
    if(shadow_trampoline||!readable(site,6)||!equal((void*)site,expected,6))return 0;
    u8* trampoline=VirtualAlloc(0,11,0x3000,0x40);
    if(!trampoline)return 0;
    copy(original,(void*)site,6);copy(trampoline,original,6);trampoline[6]=0xe9;
    put(trampoline+7,site+6-(u32)trampoline-11);
    FlushInstructionCache(GetCurrentProcess(),trampoline,11);
    if(!VirtualProtect((void*)site,6,0x40,&protection))return 0;
    image_base=base;shadow_trampoline=(u32)trampoline;
    u8 patch[6]={0xe9,0,0,0,0,0x90};put(patch+1,(u32)&shadow_enter-site-5);
    copy((void*)site,patch,6);FlushInstructionCache(GetCurrentProcess(),(void*)site,6);
    u32 ignored;VirtualProtect((void*)site,6,protection,&ignored);return 1;
}
__declspec(dllexport) void ShadowAnchor(void) {}
#ifdef MNM_SHADOW_SELFTEST
__declspec(dllexport) int WIN ShadowInstallForTest(u32 base) { return install(base); }
#endif
int WIN DllMain(void* instance,u32 reason,void* reserved) {
    (void)instance;(void)reserved;
    if(reason==1) {
        char number[8];u32 n=GetEnvironmentVariableA("MNM_SHADOW_CALLS",number,8);
        if(n && n<8) {u32 parsed=0;for(u32 i=0;i<n;++i) {if(number[i]<'0'||number[i]>'9')return 1;parsed=parsed*10+number[i]-'0';}if(parsed<=100)limit=parsed;}
        n=GetEnvironmentVariableA("MNM_SHADOW_DIR",directory,sizeof(directory));
        if(n>=sizeof(directory))return 1;
        if(!n)copy(directory,"shadow",7);
        if(limit)install((u32)GetModuleHandleA(0));
    }
    return 1; /* unsupported host = passthrough, never prevents DLL load */
}
