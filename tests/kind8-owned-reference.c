/* Actual original RNG seeding, constructors, terrain facts, custom allocation
 * and nondeleting/deleting destruction. Only original delete's checked entry
 * has a forwarding observation adapter; its actual allocator body still runs. */
#include "original-pe32-image.h"
#include "original-pe32-imports.h"
#include "kind8-owned-record.h"
static struct Kind8OwnedRecord record;
static u8 cells[192*12],descriptors[3*356],shades[96];
static u32 current_object,current_plane,free_phase,free_count;
static u32 cases,allocation_words,all_frees,reused_addresses,last_object;
static const u32 constructors[6]={0x545dc0,0x546590,0x546230,0x548560,0x548650,0x548ac0};
static const u32 classes[6]={0x5c73f0,0x5c7420,0x5c7408,0x5c7494,0x5c74b0,0x5c74cc};
static const u32 seeds[6]={0,1,0x12345678,0xffffffff,0x80000000,0xdeadbeef};
static const i32 dimensions[6]={-1,1,2,3,16,17},heights[9]={-3,0,15,31,127,255,256,512,600};
static const i32 positions[3][2]={{-129,257},{0,0},{127,95}};
static const i32 colours[4][3]={{192,0,0},{24,0,0},{255,127,65},{-1,256,-7}};
__attribute__((used)) static u32 free_continue=0x597876;
void kind8_free_observe(u32* registers){
    u32* stack=registers+9;u32 pointer=stack[1];
    original_require(free_count<4,163);
    u32 kind=pointer==current_plane&&pointer?1:pointer==current_object?2:0;
    original_require(kind!=0,164);
    record.frees[free_count*2]=kind;record.frees[free_count*2+1]=free_phase;
    ++free_count;++all_frees;
}
__attribute__((naked)) static void forwarding_delete(void){
    __asm__("pushfl\n\tpushal\n\tmovl %esp,%eax\n\tpushl %eax\n\tcall _kind8_free_observe\n\taddl $4,%esp\n\tpopal\n\tpopfl\n\tmovl 4(%esp),%eax\n\ttestl %eax,%eax\n\tjmp *_free_continue");
}
static void install_delete_observer(void){
    static const u8 expected[6]={0x8b,0x44,0x24,0x04,0x85,0xc0};
    u8* entry=(u8*)0x597870;for(u32 i=0;i<6;++i)original_require(entry[i]==expected[i],165);
    entry[0]=0xe9;*(u32*)(entry+1)=(u32)&forwarding_delete-0x597875;entry[5]=0x90;
}
static void write_record(HANDLE f,const void* p,u32 n){u32 written;original_require(WriteFile(f,p,n,&written,0)&&written==n,166);}
static void normalize(u8* out,const u8* object,u32 kind,i32 z){
    original_copy(out,object,128);
    if(kind==4){
        u32 clamped=*(u32*)(object+12);
        if(z<256&&clamped<256){
            u32 x=*(u32*)(object+4)>>5,y=*(u32*)(object+8)>>5,k=(clamped>>4)*12+y*4+x;
            original_require(*(u32*)(object+0x3e)==(u32)(cells+k*12),167);
            u32 block=*(u16*)(cells+k*12);
            original_require(*(u32*)(object+0x42)==(u32)(descriptors+block*356),168);
            *(u32*)(out+0x3e)=0x10000000+k;*(u32*)(out+0x42)=0x20000000+block;
        }
    }
    if(kind==3||kind==4){u32 plane=*(u32*)(object+0x2e);
        if(plane){original_require(plane==current_plane,169);*(u32*)(out+0x2e)=0x30000001;}}
}
void start(void){
    original_map();u32 imports=original_kernel_imports();original_require(imports>0,170);
    /* Keep the fixture's x87 precision/rounding explicit and reproducible. */
    u16 control=0x037f;__asm__("fldcw %0"::"m"(control));
    for(u32 i=0;i<250;++i)original_require(((u32*)0x5eb940)[i]==(i+1)%250,171);
    for(u32 i=0;i<192*12;++i)cells[i]=(u8)(i*7+11);
    for(u32 i=0;i<192;++i)*(u16*)(cells+12*i)=i%3;
    for(u32 i=0;i<96;++i)shades[i]=(u8)(i*37);
    *(u32*)0x6c54dc=(u32)cells;*(u32*)0x6c5c5c=(u32)shades;*(u32*)0x65660c=(u32)descriptors;
    for(u32 i=0;i<16;++i)((u32*)0x6cb8c2)[i]=i*12;
    for(u32 i=0;i<3;++i)((u32*)0x6cb942)[i]=i*4;
    *(u32*)0x5eb480=128;*(u32*)0x5eb484=96;*(u32*)0x5eb488=256;
    *(u32*)0x5e1464=19;*(u32*)0x5e1428=17;
    typedef void(__attribute__((thiscall))*Seed)(void*,u32);
    typedef u32(__attribute__((thiscall))*Next)(void*);
    HANDLE rng=CreateFileA("kind8-rng.bin",0x40000000,0,0,1,0,0);original_require(rng!=(HANDLE)-1,172);
    static u32 rng_record[256];
    for(u32 s=0;s<6;++s){
        ((Seed)0x54e170)((void*)0x6574c8,seeds[s]);
        for(u32 i=0;i<=1024;++i){
            rng_record[0]=seeds[s];rng_record[1]=i;rng_record[2]=i?((Next)0x54e200)((void*)0x6574c8):0;
            rng_record[3]=*(u32*)0x6f2a9c;original_copy(rng_record+4,(void*)0x6574c8,1008);
            write_record(rng,rng_record,sizeof(rng_record));
        }
    }CloseHandle(rng);
    install_delete_observer();
    HANDLE f=CreateFileA("kind8-owned.bin",0x40000000,0,0,1,0,0);original_require(f!=(HANDLE)-1,173);
    for(u32 kind=0;kind<6;++kind)for(u32 format=0;format<2;++format)
    for(u32 dimension=0;dimension<6;++dimension)for(u32 enabled=0;enabled<2;++enabled)
    for(u32 z=0;z<9;++z)for(u32 xy=0;xy<3;++xy)for(u32 s=0;s<6;++s)
    for(u32 deleting=0;deleting<2;++deleting)for(u32 variant=0;variant<((kind==3||kind==4)?4u:1u);++variant){
        u8* bytes=(u8*)&record;for(u32 i=0;i<sizeof(record);++i)bytes[i]=0;
        ((Seed)0x54e170)((void*)0x6574c8,seeds[s]);
        *(u32*)0x6e1f88=format;*(u32*)0x5e142c=(u32)dimensions[dimension];*(u32*)0x5e1430=enabled;
        typedef void*(*Allocate)(u32);u8* object=((Allocate)0x597850)(128);original_require(object!=0,174);
        current_object=(u32)object;current_plane=0;free_count=free_phase=0;
        if(last_object==(u32)object)++reused_addresses;last_object=(u32)object;
        for(u32 i=0;i<128;++i)object[i]=0xa5;
        typedef void*(__attribute__((thiscall))*Ctor6)(void*,i32,i32,i32,i32,i32,i32);
        typedef void*(__attribute__((thiscall))*Ctor3)(void*,i32,i32,i32);
        void* result=kind==2?((Ctor3)constructors[kind])(object,positions[xy][0],positions[xy][1],heights[z]):
            ((Ctor6)constructors[kind])(object,positions[xy][0],positions[xy][1],heights[z],-7,9,-1);
        original_require(result==object&&*(u32*)object==classes[kind],175);
        if(kind==3||kind==4){
            current_plane=*(u32*)(object+0x2e);
            if(variant){typedef void(__attribute__((thiscall))*SetColour)(void*,i32,i32,i32);
                ((SetColour)0x547890)(object+0x1c,colours[variant][0],colours[variant][1],colours[variant][2]);}
        }
        u32 words=current_plane?*(u32*)(object+0x20) * *(u32*)(object+0x24)*3:0;
        original_require(words<=768,176);allocation_words+=words;
        if(words)original_copy(record.plane,(void*)current_plane,words*2);
        normalize(record.constructed,object,kind,heights[z]);
        record.header[0]=cases++;record.header[1]=kind;record.header[2]=format;record.header[3]=(u32)dimensions[dimension];
        record.header[4]=enabled;record.header[5]=(u32)heights[z];record.header[6]=(u32)positions[xy][0];record.header[7]=(u32)positions[xy][1];
        record.header[8]=seeds[s];record.header[9]=deleting;record.header[10]=words;record.header[11]=1;record.header[15]=variant;
        original_copy(record.random,(void*)0x6574c8,1008);
        typedef void*(__attribute__((thiscall))*Dtor)(void*,u32);
        void** table=*(void***)object;record.header[12]=((Dtor)table[0])(object,deleting)==object;
        original_require(record.header[12],177);
        if(!deleting){normalize(record.nondeleted,object,kind,heights[z]);free_phase=1;typedef void(*Free)(void*);((Free)0x597870)(object);}
        record.header[13]=free_count;record.header[14]=kind==5;
        original_require(free_count==1+(words!=0),178);write_record(f,&record,sizeof(record));
    }
    original_require(cases==93312&&reused_addresses>0,179);CloseHandle(f);
    typedef u32(*FinishHeap)(void);u32 retired=((FinishHeap)0x597db0)();
    original_require(retired&&*(u32*)0x5f2094==0,180);
    u32 meta[8]={cases,allocation_words,all_frees,reused_addresses,imports,retired,0,1};
    HANDLE m=CreateFileA("kind8-owned-meta.bin",0x40000000,0,0,1,0,0);original_require(m!=(HANDLE)-1,181);
    write_record(m,meta,sizeof(meta));CloseHandle(m);ExitProcess(0);
}
