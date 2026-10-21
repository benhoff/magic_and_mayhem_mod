#pragma once
/* Recovered No-CD creature drawing state. All addresses are PE32 value tokens;
 * the caller supplies owned memory access, never host C++ object layouts.
 * A failed access refuses the operation; no original fallback is requested.
 * Integer-only code is shared by the runtime adapter and offline composition. */
typedef unsigned int MnmDrawU32;
typedef unsigned char MnmDrawU8;
typedef struct {
    void* context;
    int (*read)(void*, MnmDrawU32, void*, MnmDrawU32);
    int (*write)(void*, MnmDrawU32, const void*, MnmDrawU32);
} MnmCreatureDrawMemory;

static inline int mnm_draw_read32(MnmCreatureDrawMemory m, MnmDrawU32 p,
                                  MnmDrawU32 offset, MnmDrawU32* value) {
    MnmDrawU8 b[4];
    if (p > 0xffffffffu-offset || !m.read(m.context,p+offset,b,4)) return 0;
    *value=b[0]|(MnmDrawU32)b[1]<<8|(MnmDrawU32)b[2]<<16|(MnmDrawU32)b[3]<<24;
    return 1;
}
static inline int mnm_draw_write32(MnmCreatureDrawMemory m, MnmDrawU32 p,
                                   MnmDrawU32 offset, MnmDrawU32 value) {
    MnmDrawU8 b[4]={(MnmDrawU8)value,(MnmDrawU8)(value>>8),
                   (MnmDrawU8)(value>>16),(MnmDrawU8)(value>>24)};
    return p<=0xffffffffu-offset && m.write(m.context,p+offset,b,4);
}

/* 0051e260: unsigned definition+34 test; type18/+108 gate; target+e4
 * gates the increment, independently of the pool's active field at +4. */
static inline int mnm_creature_draw_prepare(MnmCreatureDrawMemory m,
        MnmDrawU32 creature, MnmDrawU32 pool, MnmDrawU32 count) {
    MnmDrawU32 action,subtype,type,value,definition,target,live,counter;
    MnmDrawU8 flag;
    int enabled=0;
    if (!mnm_draw_read32(m,creature,0x5ec,&action)) return 0;
    if (action==11) {
        if (!mnm_draw_read32(m,creature,0x684,&subtype)) return 0;
        enabled=subtype==1 || subtype==5;
    }
    if (action==3 || action==9 || action==10) enabled=1;
    if (!enabled) {
        if (creature>0xffffffffu-0x723 ||
            !m.read(m.context,creature+0x723,&flag,1)) return 0;
        if (flag) {
            if (!mnm_draw_read32(m,creature,0xa8,&type)) return 0;
            if (type==18) {
                if (!mnm_draw_read32(m,creature,0x108,&value)) return 0;
                if (value) goto selected;
            }
            if (!mnm_draw_read32(m,creature,0xac,&definition) ||
                !mnm_draw_read32(m,definition,0x34,&value)) return 0;
            enabled=value!=0;
        }
    }
selected:
    if (!mnm_draw_read32(m,creature,0x68,&value)) return 0;
    if (!value || !enabled) return 1;
    if (!mnm_draw_read32(m,creature,0x64c,&target)) return 0;
    if (target==0xffffffffu) return 1;
    /* The original out-of-range branch dereferences NULL+e4. Refuse it;
     * do not turn malformed references into a successful no-op. */
    if (target>=count || !pool || target>(0xffffffffu-pool)/0xe4b) return 0;
    target=pool+target*0xe4b;
    if (!mnm_draw_read32(m,target,0xe4,&live)) return 0;
    if (!live) return 1;
    return mnm_draw_read32(m,target,0x6e4,&counter) &&
           mnm_draw_write32(m,target,0x6e4,counter+1u);
}

/* 0051fa60 and real overlap-aware 0059cb60: shift before reading the
 * descriptor, which may itself alias one of the modified history records. */
static inline int mnm_creature_draw_history(MnmCreatureDrawMemory m,
        MnmDrawU32 creature, MnmDrawU32 x, MnmDrawU32 y) {
    MnmDrawU8 previous[24];
    MnmDrawU32 descriptor,value;
    if (creature>0xffffffffu-0x778 ||
        !m.read(m.context,creature+0x754,previous,24) ||
        !m.write(m.context,creature+0x760,previous,24) ||
        !mnm_draw_write32(m,creature,0x754,x) ||
        !mnm_draw_write32(m,creature,0x758,y) ||
        !mnm_draw_read32(m,creature,0xb4,&descriptor) ||
        !mnm_draw_read32(m,descriptor,4,&value)) return 0;
    return mnm_draw_write32(m,creature,0x75c,value);
}

/* 0052ab80 retains entry backing and reloads count after each iteration.
 * The caller's pool envelope is +0 pointer / +228 high-water count. */
static inline int mnm_creature_draw_finish(MnmCreatureDrawMemory m,
                                         MnmDrawU32 envelope) {
    MnmDrawU32 base,count,active;
    if (!mnm_draw_read32(m,envelope,0,&base) ||
        !mnm_draw_read32(m,envelope,0x228,&count)) return 0;
    for (MnmDrawU32 i=0;i<count;++i) {
        if (!base || i>(0xffffffffu-base)/0xe4b) return 0;
        const MnmDrawU32 creature=base+i*0xe4b;
        if (!mnm_draw_read32(m,creature,4,&active)) return 0;
        if (active && !mnm_draw_write32(m,creature,0x6e4,0)) return 0;
        if (!mnm_draw_read32(m,envelope,0x228,&count)) return 0;
    }
    return 1;
}

/* 004fd5ad..004fd5c1. Do not clear the capacity or physical queue rows. */
static inline int mnm_scene_queue_retire(MnmCreatureDrawMemory m,
                                        MnmDrawU32 renderer) {
    MnmDrawU32 base;
    return mnm_draw_read32(m,renderer,0x229,&base) &&
           mnm_draw_write32(m,renderer,0x231,0) &&
           mnm_draw_write32(m,renderer,0x22d,base);
}
