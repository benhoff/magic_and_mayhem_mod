/* Opt-in native creature drawing helpers; no original-body fallback. */
#include "../shadow/win32_min.h"
#include "../../reconstruction/rendering/creature_drawing.h"
#include "original_bytes.h"
extern void mnm_creature_prepare_entry(void),mnm_creature_history_entry(void);
extern void mnm_creature_finish_entry(void),mnm_creature_queue_entry(void);

static int access_span(u32 p,u32 n,int writing) {
    if (!p || n>0xffffffffu-p) return 0;
    const u32 end=p+n;
    while (p<end) {
        u32 region[7];
        if (VirtualQuery((void*)p,region,28)!=28 || region[4]!=0x1000 ||
            (region[5]&0x101) || !(region[5]&(writing?0xcc:0xee))) return 0;
        const u32 next=region[0]+region[3];
        if (next<=p) return 0;
        p=next<end?next:end;
    }
    return 1;
}
static int read_memory(void* context,u32 p,void* target,u32 n) {
    (void)context;
    if (!access_span(p,n,0)) return 0;
    for (u32 i=0;i<n;++i) ((u8*)target)[i]=((const u8*)p)[i];
    return 1;
}
static int write_memory(void* context,u32 p,const void* source,u32 n) {
    (void)context;
    if (!access_span(p,n,1)) return 0;
    for (u32 i=0;i<n;++i) ((u8*)p)[i]=((const u8*)source)[i];
    return 1;
}
void mnm_creature_draw_dispatch(u32* registers,u32 kind) {
    const u32 error=GetLastError();
    MnmCreatureDrawMemory memory={0,read_memory,write_memory};
    u32 pool,count;
    int ok=0;
    if (kind==0) {
        ok=mnm_draw_read32(memory,0x6def58,0,&pool) &&
           mnm_draw_read32(memory,0x6def58,4,&count) &&
           mnm_creature_draw_prepare(memory,registers[6],pool,count);
    } else if (kind==1) {
        u32 x,y;
        ok=mnm_draw_read32(memory,registers[3],8,&x) &&
           mnm_draw_read32(memory,registers[3],12,&y) &&
           mnm_creature_draw_history(memory,registers[6],x,y);
    } else if (kind==2) {
        ok=mnm_creature_draw_finish(memory,registers[6]);
    } else if (kind==3) {
        ok=mnm_scene_queue_retire(memory,registers[1]);
    }
    SetLastError(error);
    if (!ok) ExitProcess(0x43524400u+kind);
}

/* Every complete recovered body is checked before any instruction is changed.
 * Installation happens at import load, before the game's entrypoint. */
int mnm_creature_draw_install(void) {
    const u32 targets[4]={(u32)&mnm_creature_prepare_entry,
        (u32)&mnm_creature_history_entry,(u32)&mnm_creature_finish_entry,
        (u32)&mnm_creature_queue_entry};
    u32 protection[4],prepared=0;
    for (u32 i=0;i<4;++i) {
        const struct MnmCreatureOriginal* guard=&mnm_creature_original[i];
        if (!access_span(guard->address,guard->size,0)) return 0;
        for (u32 j=0;j<guard->size;++j)
            if (((u8*)guard->address)[j]!=guard->bytes[j]) return 0;
    }
    for (;prepared<4;++prepared) {
        const struct MnmCreatureOriginal* guard=&mnm_creature_original[prepared];
        if (!VirtualProtect((void*)guard->address,8,0x40,&protection[prepared])) break;
    }
    if (prepared!=4) {
        while (prepared) {
            --prepared;
            u32 ignored;
            VirtualProtect((void*)mnm_creature_original[prepared].address,8,
                           protection[prepared],&ignored);
        }
        return 0;
    }
    for (u32 i=0;i<4;++i) {
        u8 patch[8]={0xe9,0,0,0,0,0x90,0x90,0x90};
        const u32 site=mnm_creature_original[i].address;
        const u32 displacement=targets[i]-site-5;
        for (u32 j=0;j<4;++j) patch[j+1]=(u8)(displacement>>(j*8));
        for (u32 j=0;j<8;++j) ((u8*)site)[j]=patch[j];
    }
    for (u32 i=0;i<4;++i) {
        u32 ignored;
        const u32 site=mnm_creature_original[i].address;
        if (!VirtualProtect((void*)site,8,protection[i],&ignored) ||
            !FlushInstructionCache(GetCurrentProcess(),(void*)site,8))
            ExitProcess(0x43524410u);
    }
    return 1;
}
__declspec(dllexport) void CreatureDrawingAnchor(void) {}
#ifndef MNM_CREATURE_DRAW_TEST
int WIN DllMain(void* instance,u32 reason,void* reserved) {
    (void)instance;(void)reserved;
    if (reason!=1) return 1;
    const u32 error=GetLastError();
    int ok=(u32)GetModuleHandleA(0)==0x400000 && mnm_creature_draw_install();
    SetLastError(error);
    return ok;
}
#endif
