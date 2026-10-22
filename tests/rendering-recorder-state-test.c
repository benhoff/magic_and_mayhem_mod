/* Host fault injection: Win32 lock APIs may clobber LastError. */
#include <assert.h>
#include <stdlib.h>
typedef unsigned u32;
#define API
#define WIN
static u32 error,depth,locks;
static u32 GetLastError(void){return error;}
static void SetLastError(u32 value){error=value;}
static u32 GetEnvironmentVariableA(const char* key,char* value,u32 size){(void)key;(void)size;value[0]='1';return 1;}
static void ExitProcess(u32 code){exit((int)code);}
void InitializeCriticalSection(void* p){(void)p;error=123;}
void EnterCriticalSection(void* p){(void)p;++locks;error=456;}
void LeaveCriticalSection(void* p){(void)p;--locks;error=789;}
#include "../runtime/scene/producer_trace_guard.h"
int main(void){
 assert(producer_trace_configure());
 error=0xcafe;
 {PRODUCER_TRACE_SCOPE;assert(error==0xcafe&&locks==1);assert(!*producer_trace_depth(1));++*producer_trace_depth(1);error=0xbeef;}
 assert(error==0xbeef&&!locks);
 {PRODUCER_TRACE_SCOPE;assert(*producer_trace_depth(1)==1);assert(!*producer_trace_depth(2));--*producer_trace_depth(1);}
 assert(error==0xbeef&&!locks&&!depth);
 return 0;
}
