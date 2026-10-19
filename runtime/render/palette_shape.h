#ifndef MNM_RENDER_PALETTE_SHAPE_H
#define MNM_RENDER_PALETTE_SHAPE_H
/* Canonical RGB entry counts. Byte-entry palettes describe hardware indices,
 * which are deliberately not interpreted as RGB colour triples. */
static u32 game_palette_entry_count(u32 caps){
 if(caps&~0x3fdu)return 0;
 u32 size=caps&0x305u;
 return size==4?256:size==1?16:size==0x100?2:size==0x200?4:0;
}
#endif
