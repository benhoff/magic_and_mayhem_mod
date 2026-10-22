/* Pinned installed Wine only. Whole prologues contain no relocation operands. */
static u32 wine_texture_entry_enabled;
static int wine_texture_entry_mode(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_TEXTURE_ENTRY",flag,2);if(!n)return 1;
 if(n!=1||flag[0]!='1')return 0;wine_texture_entry_enabled=1;return 1;
}
static int wine_texture_entry_patch_rvas(void* module,const u32 rva[2]){
 if(!module||!rva||(u32)module>~0u-6u)return 0;
 for(u32 i=0;i<2;++i)if(rva[i]>~0u-(u32)module-6u)return 0;
 const u8 expected[2][6]={{0x55,0x89,0xe5,0x57,0x56,0x53},{0x55,0x89,0xe5,0x83,0xec,0x34}};
 void* hooks[2]={(void*)&wine_lifetime_texture_create,(void*)&wine_lifetime_texture_decref};
 u8* trampolines[2];
 for(u32 i=0;i<2;++i){u8* site=(u8*)module+rva[i];if(!readable(site,6)||!equal(site,expected[i],6))return 0;}
 for(u32 i=0;i<2;++i){u8* site=(u8*)module+rva[i];u8* out=VirtualAlloc(0,11,0x3000,0x40);if(!out)return 0;copy(out,expected[i],6);out[6]=0xe9;u32 delta=(u32)site+6-(u32)out-11;copy(out+7,&delta,4);if(!FlushInstructionCache(GetCurrentProcess(),out,11))return 0;trampolines[i]=out;}
 for(u32 i=0;i<2;++i){u8* site=(u8*)module+rva[i];u32 old,unused;if(!VirtualProtect(site,6,0x40,&old))return 0;wine_lifetime_original[i]=trampolines[i];u8 patch[6]={0xe9,0,0,0,0,0x90};u32 delta=(u32)hooks[i]-(u32)site-5;copy(patch+1,&delta,4);copy(site,patch,6);int okay=FlushInstructionCache(GetCurrentProcess(),site,6);if(!VirtualProtect(site,6,old,&unused)||!okay)return 0;}return 1;
}

static int wine_texture_entry_patch(void* module){const u32 rva[2]={0x217480,0x217e10};return wine_texture_entry_patch_rvas(module,rva);}
