#ifndef MNM_WORLD_EFFECT_RASTER_V1_H
#define MNM_WORLD_EFFECT_RASTER_V1_H
/* Tagged extension to V3/V4/V5 owned raster records. Word14 is the World wire
 * operation8..12, never the internal CompositeMode enum (additive is7 there).
 * Word15 is the pinned backend; word16 is shade; word17 is indexed. Words19/20
 * are encoded/payload lengths. The frame pointer field is normalized to zero.
 * Palette selection and effective row offsets are owned bytes, not pointers.
 * Tint stores512palette bytes +3raw RGB DWORDs; source rows store512+4*height;
 * destination rows store4*height. Other old protocols refuse the extension. */
static inline int mnm_world_effect_raster(unsigned op,unsigned backend,
        unsigned indexed,unsigned height,unsigned payload){
    if(indexed>1)return 0;
    if(op==8)return backend==13&&indexed==1&&payload==512;
    if(op==9)return backend==14&&indexed==1&&payload==512;
    if(op==10)return backend==15&&indexed==1&&payload==524;
    if(!height||height>2048)return 0;
    if(op==11)return backend==16&&indexed==1&&payload==512+height*4;
    if(op==12)return backend==7&&payload==height*4;
    return 0;
}
static inline int mnm_world_effect_source(unsigned indexed,unsigned height,unsigned payload){
    if(indexed>1)return 0;
    if(payload==0||payload==64||payload==512)return 1;
    if(indexed&&payload==524)return 1;
    return height&&height<=2048&&(payload==height*4||(indexed&&payload==512+height*4));
}
#endif
