#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/world_frame_v1.h"
#include "world_kinds.h"
extern void world_leave(void);
extern void world_wave_leave(void);
extern u8* world_stream_begin(u32*);
extern void world_stream_publish(u32,u32,u32,u32,u32,u32);
extern void world_stream_fail(u32);
extern void world_primitive_0(void),world_primitive_1(void),world_primitive_2(void),world_primitive_3(void),world_primitive_4(void),world_primitive_5(void),world_primitive_6(void),world_primitive_7(void),world_primitive_8(void),world_primitive_9(void),world_primitive_10(void),world_primitive_11(void),world_primitive_12(void);
u32 world_return,world_wave_return,world_trampolines[13];
static u32 active,failed,at,count,sequence,canvas,stride,height,streaming,startup_replay;
static u32 wave_pending,wave_amplitude,wave_phase;
static u8* inputs;
static char directory[220];
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
static int save(const char* suffix,const void* bytes,u32 n){
    char path[260];u32 i=0;while(directory[i]){path[i]=directory[i];++i;}path[i++]='\\';
    const char* prefix="world-";while(*prefix)path[i++]=*prefix++;
    for(u32 d=1000;d;d/=10)path[i++]=(char)('0'+sequence/d%10);
    while(*suffix)path[i++]=*suffix++;path[i]=0;
    HANDLE f=CreateFileA(path,0x40000000,0,0,1,0x80,0);if(f==(HANDLE)-1)return 0;
    u32 done=0;while(done<n){u32 wrote=0;if(!WriteFile(f,(const u8*)bytes+done,n-done,&wrote,0)||!wrote||wrote>n-done){CloseHandle(f);return 0;}done+=wrote;}return CloseHandle(f)!=0;
}
static int hook(u32 tag,u32 address,void* entry,u32 length,const u8* signature){
    if(world_trampolines[tag])return 1;
    if(!readable(address,length))return 0;
    for(u32 i=0;i<length;++i)if(((u8*)address)[i]!=signature[i])return 0;
    u8* trampoline=VirtualAlloc(0,length+5,0x3000,0x40);if(!trampoline)return 0;
    copy(trampoline,(void*)address,length);trampoline[length]=0xe9;put(trampoline+length+1,address-(u32)trampoline-5);
    u32 old,unused;if(!VirtualProtect((void*)address,length,0x40,&old))return 0;
    world_trampolines[tag]=(u32)trampoline;
    u8 patch[8];for(u32 i=0;i<length;++i)patch[i]=0x90;patch[0]=0xe9;put(patch+1,(u32)entry-address-5);copy((void*)address,patch,length);
    FlushInstructionCache(GetCurrentProcess(),trampoline,length+5);FlushInstructionCache(GetCurrentProcess(),(void*)address,length);VirtualProtect((void*)address,length,old,&unused);return 1;
}
static int install(void){
    const u8 normal[]={0x55,0x8b,0xec,0x56,0x57,0x53},slow[]={0x55,0x8b,0xec,0x83,0xec,0x18},half[]={0x83,0xec,0x1c,0x53,0x55,0x56},quarter[]={0x83,0xec,0x24,0x53,0x55,0x56},wave[]={0x83,0xec,0x20,0x8b,0x44,0x24,0x28};
    u32 generic=get((void*)0x6c4e3c),terrain=get((void*)0x689b7c),wave_backend=get((void*)0x689b78);
    if(wave_backend!=0x596490&&wave_backend!=0x5968a4)return 0;
    const u8 colour[]={0x83,0xec,0x24,0xa1,0x88,0x1f,0x6e,0x00};
    const u8 additive[]={0x83,0xec,0x1c,0xa1,0x88,0x1f,0x6e,0x00};
    const u8 shadow[]={0x83,0xec,0x18,0xa1,0x88,0x1f,0x6e,0x00};
    if((generic!=0x5947b2&&generic!=0x59521a)||(terrain!=0x595b47&&terrain!=0x59603e))return 0;
    return hook(0,0x595677,world_primitive_0,6,normal)&&hook(1,generic,world_primitive_1,6,normal)&&hook(2,terrain,world_primitive_2,6,normal)&&hook(3,0x57de00,world_primitive_3,6,slow)&&hook(4,0x57ec90,world_primitive_4,6,half)&&hook(5,0x57f0f0,world_primitive_5,6,quarter)&&hook(6,0x57f5f0,world_primitive_6,6,quarter)&&hook(7,0x5806f0,world_primitive_7,7,wave)&&hook(8,0x597086,world_primitive_8,6,normal)&&hook(9,wave_backend,world_primitive_9,6,normal)&&hook(10,0x57e540,world_primitive_10,8,shadow)&&hook(11,0x545c10,world_primitive_11,8,additive)&&hook(12,0x547d80,world_primitive_12,8,colour);
}
static int kind8_supported(u32 row){
    u32 object=get((void*)(row+4));
    /* Only the observed first-table route with inactive auxiliary markers.
       Other modes/auxiliary drawing remain a separate reconstruction. */
    if(!get((void*)0x5e1404)||get((void*)(row+28))!=0x8ad08ad0||get((void*)0x6e1f88)||!readable(object,72))return 0;
    u32 table=get((void*)object);
    if(!readable(table,16))return 0;u32 method=get((void*)(table+12));
    return (table==0x5c7420&&method==0x54ab00)||
        ((table==0x5c73f0||table==0x5c7408)&&method==0x546540&&readable(object,100))||
        ((table==0x5c7494||table==0x5c74b0)&&method==0x54ab60)||(table==0x5c74cc&&method==0x54ac00);
}
void world_begin(u32* registers,u32 sample){
    if(active||!GetEnvironmentVariableA("MNM_SCENE_WORLD",directory,2))return;
    streaming=sample==0;
    char mode[2];startup_replay=GetEnvironmentVariableA("MNM_SCENE_STARTUP_REPLAY",mode,2)!=0;
    u32 n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory)||(u32)GetModuleHandleA(0)!=0x400000||!install())return;
    u32 queue=registers[6];if(!readable(queue,24))return;
    u32 base=get((void*)queue),draws=get((void*)(queue+8));if(!draws||draws>12320||!readable(base,draws*36))return;
    canvas=get((void*)0x658174);stride=get((void*)0x6a2dc8);height=get((void*)0x656618);
    u32 width=get((void*)0x6a49b8);
    if(!width||width>2048||stride<width||stride>4096||!height||height>2048||!readable(canvas,stride*height*2))return;
    inputs=streaming?world_stream_begin(&sample):HeapAlloc(GetProcessHeap(),0,MNM_WORLD_MAX_BYTES);if(!inputs)return;
    zero(inputs,MNM_WORLD_HEADER);copy(inputs,MNM_WORLD_MAGIC,8);put(inputs+8,1);put(inputs+12,MNM_WORLD_HEADER);put(inputs+20,sample);put(inputs+24,width);put(inputs+28,height);put(inputs+32,stride);put(inputs+44,MNM_WORLD_BUILD);
    put(inputs+48,get((void*)0x6e0008));put(inputs+52,get((void*)0x6cbb6c));put(inputs+56,width);put(inputs+60,height);
    put(inputs+64,get((void*)0x5e1404));put(inputs+68,get((void*)0x6de6e1));put(inputs+72,get((void*)0x6e1f88));
    at=MNM_WORLD_HEADER;count=failed=0;sequence=sample;
    /* Admit only dispatch kinds whose pixel producers are all traced here.
       Unknown virtual/GDI/tint paths cannot silently become a complete frame. */
    for(u32 i=0;i<draws;++i){u32 kind=get((void*)(base+i*36+24));
        /* These first-table routes call the already traced wave primitive with
           amplitudes2/3/6. Keep this admission opt-in until startup comparison. */
        if(!world_kind_admitted(kind)&&!(kind==8&&kind8_supported(base+i*36))&&!(startup_replay&&(kind==16||kind==17||kind==20))){failed=8;break;}}
    if(!streaming&&!save(".before.565",(void*)canvas,stride*height*2)){HeapFree(GetProcessHeap(),0,inputs);inputs=0;return;}
    u32* return_slot=(u32*)(registers[3]+4);world_return=*return_slot;*return_slot=(u32)world_leave;active=1;
}
void world_observe(u32* registers,u32 tag){
    u32 error=GetLastError();
    if(!active||failed)goto done;
    u32* args=(u32*)(registers[3]+4); /* original return, then stack arguments */
    if(tag==12){
        u32 shape=registers[6];if(!readable(shape,22)||get((void*)0x6e1f88)){failed=2;goto done;}
        u32 mode=get((void*)shape);i32 w=(i32)get((void*)(shape+4)),h=(i32)get((void*)(shape+8)),x=(i32)args[1],y=(i32)args[2];
        if(w<=0||h<=0||mode>3)goto done; /* original no-op switch/default */
        if(w>64||h>64){failed=2;goto done;}
        i32 cl=(i32)get((void*)0x6e0008),ct=(i32)get((void*)0x6cbb6c),cr=(i32)get((void*)0x6a49b8),cb=(i32)get((void*)0x656618);
        if(x>=cr||y>=cb||(long long)x+w<=cl||(long long)y+h<=ct)goto done;
        u32 plane=get((void*)(shape+18)),pixels=(u32)w*(u32)h,payload=12+pixels*2,size=64+payload;
        if(plane&&!readable(plane,pixels*6)){failed=1;goto done;}
        if(count>=MNM_WORLD_MAX_DRAWS||size>MNM_WORLD_MAX_BYTES-at){failed=3;goto done;}
        if(get((void*)0x658174)!=canvas||get((void*)0x6a2dc8)!=stride){failed=5;goto done;}
        u8* record=inputs+at;zero(record,size);put(record,size);put(record+4,MNM_WORLD_COLOUR_RECT);put(record+8,(u32)x);put(record+12,(u32)y);
        put(record+16,(u32)cl);put(record+20,(u32)ct);put(record+24,(u32)cr);put(record+28,(u32)cb);put(record+40,payload);put(record+52,12);
        put(record+64,(u32)w);put(record+68,(u32)h);put(record+72,mode);
        for(u32 i=0;i<pixels;++i){const u8* rgb=(const u8*)(plane?plane+i*6:shape+12);u32 red=rgb[0]|(u32)rgb[1]<<8,green=rgb[2]|(u32)rgb[3]<<8,blue=rgb[4]|(u32)rgb[5]<<8;
            if(red>31||green>63||blue>31){failed=2;goto done;}
            u32 word=red<<11|green<<5|blue;record[76+i*2]=word;record[77+i*2]=word>>8;}
        at+=size;++count;goto done;
    }
    if(tag==11){
        u32 shape=registers[6];if(!readable(shape,20)||get((void*)0x6e1f88)){failed=2;goto done;}
        i32 w=(i32)get((void*)shape),h=(i32)get((void*)(shape+4)),x=(i32)args[1],y=(i32)args[2];
        if(w<=0||h<=0)goto done;
        if(w>2048||h>2048){failed=2;goto done;}
        i32 cl=(i32)get((void*)0x6e0008),ct=(i32)get((void*)0x6cbb6c),cr=(i32)get((void*)0x6a49b8),cb=(i32)get((void*)0x656618);
        if(x>=cr||y>=cb||(long long)x+w<=cl||(long long)y+h<=ct)goto done;
        if(count>=MNM_WORLD_MAX_DRAWS||MNM_WORLD_RECORD+20>MNM_WORLD_MAX_BYTES-at){failed=3;goto done;}
        if(get((void*)0x658174)!=canvas||get((void*)0x6a2dc8)!=stride){failed=5;goto done;}
        u8* record=inputs+at;zero(record,MNM_WORLD_RECORD+20);
        put(record,84);put(record+4,MNM_WORLD_ADDITIVE_RECT);put(record+8,(u32)x);put(record+12,(u32)y);
        put(record+16,(u32)cl);put(record+20,(u32)ct);put(record+24,(u32)cr);put(record+28,(u32)cb);
        put(record+40,20);put(record+52,11);put(record+64,(u32)w);put(record+68,(u32)h);
        for(u32 i=0;i<3;++i)put(record+72+i*4,get((void*)(shape+8+i*4))&65535);
        at+=84;++count;goto done;
    }
    u32 frame,x,y,shade=0,ordinal=0;
    if(tag<4||tag==8||tag==9){frame=args[1];x=args[2];y=args[3];if(tag>0&&tag<4){shade=args[4];ordinal=args[5];}}
    else{frame=registers[6];x=registers[5];y=args[1];if(tag!=10){shade=args[2];if(tag!=7)ordinal=args[3];}}
    if(!readable(frame,40)){failed=1;goto done;}
    u32 size=get((void*)frame),w=get((void*)(frame+4)),h=get((void*)(frame+8));
    if(size<40||size>1048576||w>2048||h>2048||!readable(frame,size)){failed=2;goto done;}
    int left=(int)x-(int)get((void*)(frame+12)),top=(int)y-(int)get((void*)(frame+16));
    int cl=(int)get((void*)0x6e0008),ct=(int)get((void*)0x6cbb6c),cr=(int)get((void*)0x6a49b8),cb=(int)get((void*)0x656618);
    /* Black/distortion entries refuse horizontal clipping. Selected generic
       and terrain backends contain their own clipped raster paths. */
    if((tag==0||tag==9)&&(left<cl||(long long)left+w>=cr))goto done;
    int coverage_top=tag==10?top+(int)(h/2):top;
    if(!w||!h||left>=cr||coverage_top>=cb||(long long)left+w<=cl||(long long)coverage_top+h<=ct)goto done;
    u32 indexed=get((void*)(frame+28))!=0xffffffff,colour_size=tag==7||tag==9?64:indexed?512:0;
    if(count>=MNM_WORLD_MAX_DRAWS||MNM_WORLD_RECORD+size+colour_size>MNM_WORLD_MAX_BYTES-at){failed=3;goto done;}
    u8* record=inputs+at;zero(record,MNM_WORLD_RECORD);
    put(record,MNM_WORLD_RECORD+size+colour_size);put(record+4,tag==4?MNM_WORLD_HALF:tag==5?4:tag==6?MNM_WORLD_QUARTER:tag==7||tag==9?MNM_WORLD_WAVE:tag==10?MNM_WORLD_PROJECTED_SHADOW:MNM_WORLD_COPY);
    put(record+8,x);put(record+12,y);put(record+16,cl);put(record+20,ct);put(record+24,cr);put(record+28,cb);put(record+32,size);put(record+36,indexed);put(record+40,colour_size);put(record+44,shade);put(record+48,get((void*)0x6c4830));put(record+52,tag);put(record+56,ordinal);
    copy(record+MNM_WORLD_RECORD,(void*)frame,size);put(record+MNM_WORLD_RECORD+28,0);
    if(tag==9){u8* offsets=record+MNM_WORLD_RECORD+size;u32 phase=get((void*)0x6c4830);int skip=top<ct?ct-top:0;put(record+60,16);for(u32 i=0;i<16;++i)put(offsets+i*4,get((void*)(0x656640+((i+phase+1-(u32)skip)&15)*4)));}
    else if(tag==7){u8* offsets=record+MNM_WORLD_RECORD+size;u32 phase=get((void*)0x6c4830);
        if(left<cl||(long long)left+w>cr){put(record+60,16);for(u32 i=0;i<16;++i){int offset=8-(int)((i+phase)&15);put(offsets+i*4,offset<0?-offset:offset);}}
        else{u32 amplitude=shade?shade:get((void*)0x5e15f8);if(!amplitude||amplitude>8){failed=7;goto done;}
            put(record+60,amplitude*2);
            if(get((void*)0x5f14d0)==0x7d00){
                if(!startup_replay||wave_pending){failed=7;goto done;}
                /* Original execution initializes its own table. Observe that
                   table after this primitive returns; never execute or predict
                   the initializer inside the observation callback. */
                wave_pending=at;wave_amplitude=amplitude;wave_phase=phase;
                world_wave_return=args[0];args[0]=(u32)world_wave_leave;
            }else for(u32 i=0;i<16;++i)put(offsets+i*4,get((void*)(0x5f14d0+(amplitude*16+(i+phase)%(amplitude*2))*4)));}
    }else if(colour_size){u8* colours=record+MNM_WORLD_RECORD+size;
        /* Only explicit black producers synthesize zero. A clipped fallback
           with shade -127 still selects the primitive's real palette. */
        if(tag==0||tag==10)zero(colours,512);
        else{u32 node=get((void*)(frame+28));
            if(ordinal>18){failed=4;goto done;}
            for(u32 i=0;i<ordinal;++i){if(!readable(node,12)){failed=4;goto done;}node=get((void*)node);}
            if(!readable(node,12)){failed=4;goto done;}
            u32 shift=get((void*)(node+4)),neutral=get((void*)(node+8));if(shift>7||neutral>127){failed=4;goto done;}
            int shifted=(int)shade>>shift;if(shifted<0&&shift)++shifted;int table=(int)neutral+shifted;
            u32 entry_size=tag==2?4:2,table_size=entry_size*256;
            if(table<0||table>254||!readable(node+12+(u32)table*table_size,table_size)){failed=4;goto done;}
            const u8* selected=(const u8*)(node+12+(u32)table*table_size);
            for(u32 i=0;i<256;++i){colours[i*2]=selected[i*entry_size];colours[i*2+1]=selected[i*entry_size+1];}
        }
    }
    if(get((void*)0x658174)!=canvas||get((void*)0x6a2dc8)!=stride){failed=5;goto done;}
    at+=MNM_WORLD_RECORD+size+colour_size;++count;
done:SetLastError(error);
}
void world_wave_complete(void){
    u32 error=GetLastError();
    if(!active||!wave_pending||get((void*)0x5f14d0)==0x7d00){failed=7;goto done;}
    u8* record=inputs+wave_pending;u8* offsets=record+MNM_WORLD_RECORD+get(record+32);
    for(u32 i=0;i<16;++i)put(offsets+i*4,get((void*)(0x5f14d0+(wave_amplitude*16+(i+wave_phase)%(wave_amplitude*2))*4)));
done:wave_pending=0;SetLastError(error);
}
#ifdef MNM_SCENE_STARTUP_REPLAY_SELFTEST
/* Synthetic active return state; no original renderer code is executed here. */
static u8 test_wave[248];
static u8 test_palette[2048];
static u8 test_additive[256];
int world_additive_test_setup(void){
    u32 object=0x600000,row=0x630000;zero((void*)object,72);zero((void*)row,36);
    put((void*)0x5e1404,1);put((void*)0x6e1f88,0);put((void*)object,0x5c7420);put((void*)0x5c742c,0x54ab00);
    put((void*)(row+4),object);put((void*)(row+28),0x8ad08ad0);
    if(!kind8_supported(row))return 0;
    put((void*)0x5e1404,0);if(kind8_supported(row))return 0;put((void*)0x5e1404,1);
    put((void*)0x6e1f88,1);if(kind8_supported(row))return 0;put((void*)0x6e1f88,0);
    put((void*)(row+28),0);if(kind8_supported(row))return 0;put((void*)(row+28),0x8ad08ad0);
    put((void*)0x5c742c,0x54ab01);if(kind8_supported(row))return 0;put((void*)0x5c742c,0x54ab00);
    put((void*)object,0x5c7424);if(kind8_supported(row))return 0;put((void*)object,0x5c7420);
    put((void*)(row+4),1);if(kind8_supported(row))return 0;put((void*)(row+4),object);
    canvas=0x700000;stride=32;put((void*)0x658174,canvas);put((void*)0x6a2dc8,stride);
    put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);put((void*)0x6a49b8,32);put((void*)0x656618,12);
    put((void*)(object+48),2);put((void*)(object+52),3);put((void*)(object+56),0x1000c);put((void*)(object+60),65535);put((void*)(object+64),6);
    zero(test_additive,sizeof(test_additive));inputs=test_additive;active=1;failed=count=0;at=80;return 1;
}
int world_colour_test_setup(void){
    if(!world_additive_test_setup())return 0;
    u32 object=0x600000,row=0x630000;put((void*)object,0x5c7494);put((void*)0x5c74a0,0x54ab60);if(!kind8_supported(row))return 0;
    put((void*)object,0x5c74b0);put((void*)0x5c74bc,0x54ab60);if(!kind8_supported(row))return 0;
    put((void*)object,0x5c73f0);put((void*)0x5c73fc,0x546540);if(!kind8_supported(row))return 0;
    put((void*)object,0x5c7408);put((void*)0x5c7414,0x546540);if(!kind8_supported(row))return 0;
    put((void*)object,0x5c74cc);put((void*)0x5c74d8,0x54ac00);if(!kind8_supported(row))return 0;
    put((void*)(object+28),2);put((void*)(object+32),2);put((void*)(object+36),3);
    u8* channels=(u8*)(object+40);channels[0]=12;channels[1]=0;channels[2]=7;channels[3]=0;channels[4]=5;channels[5]=0;put((void*)(object+46),0);return 1;
}
int world_colour_test_check(void){
    active=0;inputs=0;if(failed||count!=1||at!=168||get(test_additive+84)!=7||get(test_additive+132)!=12||get(test_additive+144)!=2||get(test_additive+148)!=3||get(test_additive+152)!=2)return 0;
    for(u32 i=0;i<6;++i)if((u32)(test_additive[156+i*2]|test_additive[157+i*2]<<8)!=(12u<<11|7u<<5|5u))return 0;return 1;
}
int world_additive_test_check(void){
    active=0;inputs=0;
    return !failed&&count==1&&at==164&&get(test_additive+80)==84&&get(test_additive+84)==6&&
        get(test_additive+88)==4&&get(test_additive+92)==5&&get(test_additive+132)==11&&
        get(test_additive+144)==2&&get(test_additive+148)==3&&get(test_additive+152)==12&&get(test_additive+156)==65535&&get(test_additive+160)==6;
}
void world_wave_test_setup(void){
    zero(test_wave,sizeof(test_wave));inputs=test_wave;active=1;failed=0;
    wave_pending=80;wave_amplitude=3;wave_phase=5;put(inputs+80+32,40);
}
int world_palette_test(void){
    u32 frame=0x600000,node=0x610000;
    zero((void*)frame,40);put((void*)frame,40);put((void*)(frame+4),1);
    put((void*)(frame+8),2);put((void*)(frame+28),node);
    put((void*)node,0);put((void*)(node+4),0);put((void*)(node+8),127);
    for(u32 i=0;i<512;++i){((u8*)(node+12))[i*2]=0x34;((u8*)(node+12))[i*2+1]=0x12;}
    canvas=0x700000;stride=4;put((void*)0x658174,canvas);put((void*)0x6a2dc8,stride);
    put((void*)0x6e0008,0);put((void*)0x6cbb6c,0);
    put((void*)0x6a49b8,4);put((void*)0x656618,4);
    for(u32 tag=0;tag<=10;++tag){
        if(tag>=7&&tag<=9)continue;
        u32 args[6]={0,frame,1,0,(u32)-127,0},regs[8]={0};
        if(tag>=4){args[1]=0;args[2]=(u32)-127;args[3]=0;}
        regs[3]=(u32)args-4;regs[6]=frame;regs[5]=1;
        zero(test_palette,sizeof(test_palette));inputs=test_palette;
        active=1;failed=count=0;at=80;wave_pending=0;
        SetLastError(1234);world_observe(regs,tag);
        if(GetLastError()!=1234||failed||count!=1)return 0;
        u8* colours=inputs+80+MNM_WORLD_RECORD+40;
        u32 expected=tag==0||tag==10?0:0x1234;
        for(u32 i=0;i<256;++i)if((u32)(colours[i*2]|colours[i*2+1]<<8)!=expected)return 0;
    }
    active=0;inputs=0;return 1;
}
int world_wave_test_check(void){
    if(wave_pending||failed)return 0;
    for(u32 i=0;i<16;++i)if(get(test_wave+184+i*4)!=13*(48+(i+5)%6))return 0;
    return 1;
}
#endif
void world_complete(u32* registers){
    u32 error=GetLastError();active=0;
    if(wave_pending){failed=7;wave_pending=0;}
    if(get((void*)0x658174)!=canvas||get((void*)0x6a2dc8)!=stride||!readable(canvas,stride*height*2))failed=5;
    // Retain a post-consumer diagnostic even for refused startup requests. Its
    // pixels are never an input to the native renderer or a successful trace.
    if(!streaming&&readable(canvas,stride*height*2)&&!save(".after.565",(void*)canvas,stride*height*2)&&!failed)failed=6;
    put(inputs+16,at);put(inputs+36,count);put(inputs+40,failed);put(inputs+76,registers[7]);
    if(streaming)world_stream_publish(at,failed,canvas,get(inputs+24),height,stride);
    else{save(".bin",inputs,at);HeapFree(GetProcessHeap(),0,inputs);}
    inputs=0;SetLastError(error);
}
