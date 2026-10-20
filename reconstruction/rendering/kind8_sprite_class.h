#ifndef MNM_KIND8_SPRITE_CLASS_H
#define MNM_KIND8_SPRITE_CLASS_H
#include <stdint.h>
/* Build-pinned identities. Numeric values never become native host pointers. */
struct MnmKind8SpriteClass {uint32_t kind,method,owner_offset,root_bytes,bank_global;};
static inline struct MnmKind8SpriteClass mnm_kind8_sprite_class(uint32_t table,uint32_t target){
    struct MnmKind8SpriteClass result={0,0,0,0,0};
    if(table==0x5c74e4 && target==0x5492b0){result.kind=1;result.method=target;result.owner_offset=0x1c;result.root_bytes=32;result.bank_global=0x6f2150;}
    else if(table==0x5c74fc && target==0x549730){result.kind=2;result.method=target;result.owner_offset=0x10;result.root_bytes=20;result.bank_global=0x6f2268;}
    return result;
}
#endif
