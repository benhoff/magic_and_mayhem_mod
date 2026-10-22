#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
void start(void){
 require(initialize(),100);u8* code=VirtualAlloc(0,256,0x3000,0x40);require(code!=0,101);
 /* Exact create prologue, write ninth cdecl argument, restore saved registers. */
 const u8 create[]={0x55,0x89,0xe5,0x57,0x56,0x53,0x8b,0x45,0x28,0xc7,0x00,0x00,0x20,0,0,0x31,0xc0,0x5b,0x5e,0x5f,0xc9,0xc3};
 /* Exact decref prologue, return zero and release local frame. */
 const u8 decref[]={0x55,0x89,0xe5,0x83,0xec,0x34,0x31,0xc0,0xc9,0xc3};
 copy(code,create,sizeof(create));copy(code+64,decref,sizeof(decref));const u32 rvas[2]={0,64};
 code[64]=0x90;require(!wine_texture_entry_patch_rvas(code,rvas)&&code[0]==0x55,102);code[64]=0x55;
 require(!wine_texture_entry_patch_rvas(0,rvas)&&!wine_texture_entry_patch_rvas(code,0)&&!wine_texture_entry_patch_rvas((void*)0xfffffffeu,rvas),108);
 const u32 wrapped[2]={0xffffffffu,64};require(!wine_texture_entry_patch_rvas(code,wrapped)&&code[0]==0x55&&code[64]==0x55,109);
 require(wine_texture_entry_patch_rvas(code,rvas),103);
 typedef u32(__attribute__((cdecl))*Create)(void*,const void*,u32,u32,u32,const void*,void*,const void*,void**);
 typedef u32(__attribute__((cdecl))*Release)(void*);
 for(u32 cycle=0;cycle<2;++cycle){void* output=0;SetLastError(0xbeef);require(((Create)code)(0,0,1,1,0,0,0,0,&output)==0&&output==(void*)0x2000&&GetLastError()==0xbeef,104);require(wine_lifetime_find(output,1)==cycle+1,105);SetLastError(0xbeef);require(((Release)(code+64))(output)==0&&GetLastError()==0xbeef&&!wine_lifetime_find(output,1),106);}
 require(wine_lifetime_generations==2,107);wine_lifetime_close(1);ExitProcess(0);
}
