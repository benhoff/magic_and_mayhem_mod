/* Pinned No-CD fading particle method54a4a0 under primary kind8 only.
 * Every byte of the original method remains a dependency. Guest pointers never
 * enter a host packet; existing CPS acquisition validates the owned source. */
static const struct FadingParticleSite {u32 address,tag_mask,parent_word;} fading_particle_sites[]={
 {0x54a509,0x00400000,8}, /* raw WORD; tag36 is handled explicitly below */
 {0x54a52b,0x00001000,8}, /* black */
 {0x54a543,0x00020000,10}, /* black AX1 clipped fallback */
 {0x54a563,0x00006000,10}, /* CPU-selected indexed */
 {0x54a57a,0x00020000,10}, /* indexed AX1 clipped fallback */
 {0x54a5a1,0x00080000,8}, /* 75 percent */
 {0x54a5c5,0x00040000,8}, /* 50 percent */
 {0x54a5e4,0x00100000,8}, /* 25 percent */
};
static inline int fading_particle_caller(u32 caller,u32 tag,const u32* stack){
 const struct FadingParticleSite* site=0;
 for(u32 i=0;i<sizeof(fading_particle_sites)/sizeof(fading_particle_sites[0]);++i)
  if(caller==fading_particle_sites[i].address){site=fading_particle_sites+i;break;}
 if(!site||tag>=64||!((tag<32&&(site->tag_mask&(1u<<tag)))||(caller==0x54a509&&tag==36)))return 0;
 if(!readable(0x54a4a0,334)||!readable(0x5006a2,16)||!readable(0x5006b2,10)||!readable((u32)stack,(site->parent_word+1)*4))return 0;
 u32 hash=2166136261u;for(u32 i=0;i<334;++i)hash=(hash^((const u8*)0x54a4a0)[i])*16777619u;
 if(hash!=0x0e95378bu)return 0;
 const u8 prefix[16]={0x8b,0x56,0x0c,0x8b,0x4e,0x04,0x52,0x8b,0x56,0x08,0x8b,0x01,0x52,0xff,0x50,0x0c};
 const u8 tail[10]={0xe9,0x91,0x01,0x00,0x00,0x0f,0xbf,0x4e,0x12,0x0f};
 for(u32 i=0;i<16;++i)if(((const u8*)0x5006a2)[i]!=prefix[i])return 0;
 for(u32 i=0;i<10;++i)if(((const u8*)0x5006b2)[i]!=tail[i])return 0;
 if(stack[site->parent_word]!=0x5006b2)return 0;
 /* Four saved registers: the enclosing queue row is saved ESI, three words
  * before the virtual return. The random sprite methods save only three. */
 const u32 row=stack[site->parent_word-3];
 if(!readable(row,36)||get((void*)(row+24))!=8||get((void*)(row+28))!=0x8ad08ad0||!get((void*)0x5e1404)||get((void*)0x6e1f88))return 0;
 const u32 object=get((void*)(row+4));
 if(!readable(object,122)||get((void*)object)!=0x5c752c||!readable(0x5c752c,16)||get((void*)0x5c7538)!=0x54a4a0)return 0;
 const u32 frame=get((void*)(object+0x5a));if(!readable(frame,32))return 0;
 const u32 words=get((void*)(frame+28))==0xffffffffu;
 if((caller==0x54a509)!=words)return 0;
 /* Stack entries have source as their first cdecl argument. Blend entries
  * pass it in ECX; their exact unchanged method and enclosing row bind it. */
 return tag>=18&&tag<=20 ? 1 : stack[1]==frame;
}
