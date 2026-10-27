/* Supplied requests exercise the real owned PE32 facade and bridge observer.
 * No original caller execution or old-run coordinate reconstruction is claimed. */
#define start fixture_existing_start
#include "native-gpu-copy-plan-probe.c"
#undef start

static u16* refusal_pixels(void* object,u32 width,u32 height,u32* pitch,u32 initialize){
    u32 descriptor[27]={108};fixture_require(!((Lock)fixture_table(object)[25])(object,0,descriptor,1,0),71);
    u8* bytes=(u8*)descriptor[9];*pitch=descriptor[4];
    for(u32 y=0;y<height;++y)for(u32 x=0;x<width;++x){
        u16* p=(u16*)(bytes+y*(*pitch))+x;u16 expected=(u16)(0x1500+x*3+y*17);
        if(initialize)*p=expected;else fixture_require(*p==expected,72);
    }
    fixture_require(!((Unlock)fixture_table(object)[32])(object,bytes),73);return (u16*)bytes;
}
void start(void){
    void* module=LoadLibraryA("MnmDraw.dll");fixture_require(module!=0,74);
    CreateDraw create=(CreateDraw)GetProcAddress(module,"NativeDrawCreate");fixture_require(create!=0,75);
    void* draw=0;fixture_require(!create(0,&draw,0),76);
    lock_capture_path[0]='.';lock_capture_path_length=1;
    game_copy_routes_v2=HeapAlloc(GetProcessHeap(),8,sizeof(*game_copy_routes_v2));fixture_require(game_copy_routes_v2!=0,77);
    game_copy_routes_v2->header[15]=0x400000;game_copy_routes_v2->header[16]=0x200000;
    static const u32 rows[][10]={
        {640,480,40,40,640,512,680,552,0,0},
        {400,280,640,480,0,0,40,40,640,512},
        {400,280,640,480,0,0,40,40,0xffffffffu,0},
        {400,280,640,480,0,0,40,40,0,0x80000000u},
        {640,480,40,40,640,512,680,552,7,9}
    };
    HANDLE file=CreateFileA("refusals.bin",0x40000000,0,0,1,0,0);fixture_require(file!=(HANDLE)-1,78);
    u32 bytes=0;
    for(u32 index=0;index<sizeof(rows)/sizeof(rows[0]);++index){
        const u32* row=rows[index];fixture_input.width=row[0];fixture_input.height=row[1];void* source=fixture_surface(draw);
        fixture_input.width=row[2];fixture_input.height=row[3];void* target=fixture_surface(draw);
        u32 source_pitch,target_pitch;u16* sb=refusal_pixels(source,row[0],row[1],&source_pitch,1);
        u16* db=refusal_pixels(target,row[2],row[3],&target_pitch,1);
        fixture_input.width=row[0];fixture_input.height=row[1];fixture_owned(source,sb,source_pitch,0,0);
        fixture_input.width=row[2];fixture_input.height=row[3];fixture_owned(target,db,target_pitch,0,0);
        struct GameBlit p;i32 sr[4];copy(sr,row+4,16);
        game_blit_before(target,0,source,sr,index==1?17:16,0,1,row[8],row[9],&p);
        fixture_require(!p.valid,79);
        fixture_require(same(p.routes+24,sr,16),80);
        const u32 expected[4]={row[8],row[9],row[8]+row[6]-row[4],row[9]+row[7]-row[5]};
        fixture_require(same(p.routes+28,expected,16),81);
        const i32 result=((i32(WIN*)(void*,u32,u32,void*,const i32*,u32))fixture_table(target)[7])(target,row[8],row[9],source,sr,index==1?17:16);
        fixture_require((u32)result==0x88760096u,82);
        refusal_pixels(source,row[0],row[1],&source_pitch,0);refusal_pixels(target,row[2],row[3],&target_pitch,0);
        p.caller=index==0?0x58c488:0x58cbb2;game_copy_routes_finish(&p,result,0);
        const u32* record=game_copy_routes_v2->rows[index];fixture_require(!record[14]&&!record[15]&&record[13]==0x88760096u&&!record[40]&&!record[41]&&!record[42]&&!record[43],83);
        fixture_require(WriteFile(file,record,176,&bytes,0)&&bytes==176,84);
        for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object)game_surface_drop(game_surfaces+i);
        fixture_release(target);fixture_release(source);
    }
    CloseHandle(file);HeapFree(GetProcessHeap(),0,game_copy_routes_v2);game_copy_routes_v2=0;fixture_release(draw);
    u32(WIN* owners)(void)=(u32(WIN*)(void))GetProcAddress(module,"NativeDrawObjects");fixture_require(owners&&owners()==0&&game_surface_bytes==0,85);
    ExitProcess(0);
}
