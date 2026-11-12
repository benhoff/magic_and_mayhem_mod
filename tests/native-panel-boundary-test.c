/* Actual menu admission helper with controlled facade lookup/results.
 * This tests routing/refusal/error preservation, not the GPU transfer itself. */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
typedef uint32_t u32;
#define WIN
static u32 error,lookups,calls,argument,result=1;
static int present=1,available=1,preferences;
static u32 GetLastError(void){return error;}
static void SetLastError(u32 value){error=value;}
static void* GetModuleHandleA(const char* name){assert(!strcmp(name,"MnmRender.dll"));++lookups;error=0x11;return present?(void*)(uintptr_t)1:0;}
static u32 storage(u32 action){++calls;argument=action;error=0x22;return result;}
static u32 settings(void){++calls;preferences=1;error=0x22;return result;}
static void* GetProcAddress(void* module,const char* name){
 assert(module==(void*)(uintptr_t)1);error=0x33;
 assert(!strcmp(name,"RenderPrepareStorageV1")||!strcmp(name,"RenderPreparePreferencesV1"));
 if(!available)return 0;
 return !strcmp(name,"RenderPreparePreferencesV1")?(void*)&settings:(void*)&storage;
}
#include "../runtime/menu/native_panel_boundary.h"
int main(void){
 unsigned checks=0;
 for(u32 consumer=0;consumer<3;++consumer)for(int kind=0;kind<4;++kind){
  error=0xabc123;lookups=calls=preferences=0;argument=99;
  present=kind!=0;available=kind!=1;result=kind!=2;
  const int ok=mini_prepare_native_panel(consumer);
  assert(error==0xabc123 && lookups==1);
  if(kind==0){assert(ok&&calls==0);}
  else if(kind==1||kind==2){assert(!ok&&calls==(u32)(kind==2));}
  else{assert(ok&&calls==1);}
  if(calls){assert(preferences==(consumer==2));if(!preferences)assert(argument==consumer);}
  ++checks;
 }
 error=0xabc123;lookups=calls=0;assert(!mini_prepare_native_panel(3)&&!lookups&&!calls&&error==0xabc123);++checks;
 printf("{\"success\":true,\"routing_refusal_controls\":%u,\"caller_error_preserved\":true}\n",checks);
 return 0;
}
