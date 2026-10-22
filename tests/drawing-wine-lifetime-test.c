#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls,result_value;static void* returned_object;
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 result(void){require(GetLastError()==0xbeef,81);++calls;SetLastError(0xcafe);return result_value;}
static u32 __attribute__((cdecl)) texture(void* device,const void* desc,u32 layers,u32 levels,u32 flags,const void* data,void* parent,const void* ops,void** output){
 require(device==(void*)1&&desc==(void*)2&&layers==3&&levels==4&&flags==5&&data==(void*)6&&parent==(void*)7&&ops==(void*)8,82);if((i32)result_value>=0)*output=returned_object;return result();
}
static u32 __attribute__((cdecl)) swapchain(void* device,const void* desc,void* state,void* parent,const void* ops,void** output){
 require(device==(void*)1&&desc==(void*)2&&state==(void*)3&&parent==(void*)4&&ops==(void*)5,83);if((i32)result_value>=0)*output=returned_object;return result();
}
static u32 __attribute__((cdecl)) decref(void* object){require(object==returned_object,84);return result();}
void start(void){
 require(initialize(),80);wine_lifetime_original[0]=(void*)&texture;wine_lifetime_original[1]=(void*)&decref;wine_lifetime_original[2]=(void*)&swapchain;wine_lifetime_original[3]=(void*)&decref;
 for(u32 kind=1;kind<=3;kind+=2){returned_object=(void*)(0x1000+kind);void* output=0;
  for(u32 cycle=0;cycle<2;++cycle){result_value=0;SetLastError(0xbeef);u32 before=calls;
   u32 r=kind==1?wine_lifetime_texture_create((void*)1,(void*)2,3,4,5,(void*)6,(void*)7,(void*)8,&output):wine_lifetime_swapchain_create((void*)1,(void*)2,(void*)3,(void*)4,(void*)5,&output);
   require(r==0&&calls==before+1&&output==returned_object&&GetLastError()==0xcafe,85);u32 generation=wine_lifetime_find(output,kind);require(generation!=0,86);
   result_value=1;SetLastError(0xbeef);r=kind==1?wine_lifetime_texture_decref(output):wine_lifetime_swapchain_decref(output);require(r==1&&wine_lifetime_find(output,kind)==generation,87);
   result_value=0;SetLastError(0xbeef);r=kind==1?wine_lifetime_texture_decref(output):wine_lifetime_swapchain_decref(output);require(!r&&!wine_lifetime_find(output,kind)&&!wine_lifetime_find(0,kind),88);
  }
  result_value=0x80004005;SetLastError(0xbeef);output=0;
  u32 r=kind==1?wine_lifetime_texture_create((void*)1,(void*)2,3,4,5,(void*)6,(void*)7,(void*)8,&output):wine_lifetime_swapchain_create((void*)1,(void*)2,(void*)3,(void*)4,(void*)5,&output);require(r==result_value&&!output,89);
 }
 require(wine_lifetime_generations==4&&calls==14,90);wine_lifetime_close(1);ExitProcess(0);
}
