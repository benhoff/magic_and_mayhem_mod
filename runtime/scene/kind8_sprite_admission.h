#include "../../reconstruction/rendering/kind8_sprite_class.h"
/* Called only inside the protected batch on its original owner thread. All
 * guest pointers stay within this PE32 process; the host receives owned CPS. */
static inline int kind8_sprite_caller(u32 caller,u32 tag,const u32* stack){
 const struct SpriteRasterSite* site=mnm_world_sprite_site(caller);
 if(!site||tag>=64||!(site->tags&(1ull<<tag)))return 0;
 const u32 n=(site->parent_word+1)*4;
 if(!readable(caller,10)||!readable(site->method,9)||!readable(0x5006a2,16)||!readable(0x5006b2,10)||!readable((u32)stack,n))return 0;
 if(stack[site->parent_word]!=0x5006b2||!mnm_world_sprite_signatures(site,tag,(const u8*)caller,(const u8*)site->method,(const u8*)0x5006a2,(const u8*)0x5006b2))return 0;
 const u32 row=stack[site->parent_word-2];
 if(!readable(row,36)||get((void*)(row+24))!=8||get((void*)(row+28))!=0x8ad08ad0||!get((void*)0x5e1404)||get((void*)0x6e1f88))return 0;
 const u32 object=get((void*)(row+4));if(!readable(object,4))return 0;
 const u32 table=get((void*)object);const struct MnmKind8SpriteClass type=mnm_kind8_sprite_class(table,site->method);
 if(!type.kind||table!=site->table||type.bank_global!=site->bank||!readable(table,16)||get((void*)(table+12))!=site->method||!readable(object,type.root_bytes))return 0;
 const u32 player=get((void*)0x644520);if(player>=8)return 0;
 const u32 owner=get((void*)(object+type.owner_offset));
 if(!readable(owner,68)||!get((void*)(owner+0x24+player*4)))return 0;
 const u32 bank=get((void*)type.bank_global);if(!readable(bank,16))return 0;
 const u32 count=get((void*)(bank+12));
 if(count<1||count>4096||!readable(stack[1],32))return 0;
 const u32 selector=get((void*)(stack[1]+28));
 return (site->parent_word==7)==(selector==0xffffffffu);
}
