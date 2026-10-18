/* Public Wine11.16 mutator entry coverage, enabled only with reservations.
 * These whole-instruction prefixes have no relocation operands. This does not
 * intercept raw stores, private workers, or prove distinct-resource disjointness.
 * Install before forwarding the first DirectDraw create; failure aborts launch. */
static int wine_mutator_entry_patch(void* module){
 if(!wine_reserve_enabled)return 1;
 const u32 rva[4]={0x204490,0x204bc0,0x1df0d0,0x221460};
 const u32 size[4]={7,6,6,7};
 const u8 prefix[4][7]={{0x55,0x89,0xe5,0x53,0x83,0xec,0x24},
  {0x55,0x89,0xe5,0x83,0xec,0x18,0},
  {0x55,0x89,0xe5,0x57,0x56,0x53,0},
  {0x8d,0x4c,0x24,0x04,0x83,0xe4,0xf8}};
 void** originals[4]={wine_resource_original+1,wine_resource_original+2,
  &wine_copy_original,&wine_source_version_rebind_original};
 void* hooks[4]={(void*)&wine_resource_map,(void*)&wine_resource_unmap,
  (void*)&wine_copy_region,(void*)&wine_source_version_rebind};
 u8* trampolines[4];
 /* Validate every site and the installed import originals before changing any. */
 for(u32 i=0;i<4;++i){u8* site=(u8*)module+rva[i];
  if(*originals[i]!=site||!readable(site,size[i])||!equal(site,prefix[i],size[i]))return 0;
 }
 for(u32 i=0;i<4;++i){u8* site=(u8*)module+rva[i];
  u8* out=VirtualAlloc(0,size[i]+5,0x3000,0x40);if(!out)return 0;
  copy(out,prefix[i],size[i]);out[size[i]]=0xe9;
  u32 delta=(u32)site-(u32)out-5;copy(out+size[i]+1,&delta,4);
  if(!FlushInstructionCache(GetCurrentProcess(),out,size[i]+5))return 0;
  trampolines[i]=out;
 }
 for(u32 i=0;i<4;++i){u8* site=(u8*)module+rva[i];u32 old,unused;
  if(!VirtualProtect(site,size[i],0x40,&old))return 0;
  *originals[i]=trampolines[i];u8 patch[7]={0xe9,0,0,0,0,0x90,0x90};
  u32 delta=(u32)hooks[i]-(u32)site-5;copy(patch+1,&delta,4);copy(site,patch,size[i]);
  int okay=FlushInstructionCache(GetCurrentProcess(),site,size[i]);
  if(!VirtualProtect(site,size[i],old,&unused)||!okay)return 0;
 }
 return 1;
}
