#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
typedef uint32_t u32;
struct NsState {void* pixels;u32 pitch,height;struct NsState* next_owned;};
static struct NsState* ns_owned;
#include "../runtime/native-draw/history_lookup.h"
#define check(v) do{if(!(v)){fprintf(stderr,"failed line%d\n",__LINE__);abort();}++checks;}while(0)
static unsigned checks;
int main(void){
 struct NsState a={(void*)0x1000,32,3,0},b={(void*)0x2000,64,2,0};a.next_owned=&b;ns_owned=&a;
 check(ns_history_find((void*)0x1000,1)==&a);check(ns_history_find((void*)0x105f,1)==&a);check(!ns_history_find((void*)0x1060,1));
 check(!ns_history_find((void*)0xfff,1));check(!ns_history_find(0,1));
 for(unsigned i=0;i<1000;++i){check(ns_history_find((void*)0x1001,1)==&a);check(ns_history_find((void*)0x2001,1)==&b);check(!ns_history_find((void*)0x3001,1));}
 /* A negative lookup cannot survive publication of an allocation there. */
 ns_history_lookup_clear();b.pixels=(void*)0x3000;check(ns_history_find((void*)0x3001,1)==&b);check(!ns_history_find((void*)0x2001,1));
 /* Physical Flip invalidates both positive and negative keys before mutation. */
 ns_history_lookup_clear();b.pixels=a.pixels;a.pixels=(void*)0x3000;
 check(ns_history_find((void*)0x3001,1)==&a);check(ns_history_find((void*)0x1001,1)==&b);
 /* Overlapping namespaces retain first-match selection, including reordering. */
 ns_history_lookup_clear();a.pixels=(void*)0x1000;check(ns_history_find((void*)0x1001,1)==&a);
 ns_history_lookup_clear();a.next_owned=0;b.next_owned=&a;ns_owned=&b;check(ns_history_find((void*)0x1001,1)==&b);
 /* Compatibility callers resolve directly and never publish shared cache keys. */
 ns_history_lookup_clear();check(ns_history_find((void*)0x1001,0)==&b);
 ns_owned=&a;check(ns_history_find((void*)0x1001,0)==&a);
 for(unsigned i=0;i<8;++i)check(!ns_history_lookup[i].address);
 /* No retired metadata may be touched after clearing an already-hot entry. */
 size_t bytes=(size_t)sysconf(_SC_PAGESIZE);struct NsState* dead=mmap(0,bytes,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);check(dead!=MAP_FAILED);
 *dead=a;ns_history_lookup_clear();ns_owned=dead;check(ns_history_find((void*)0x1001,1)==dead);
 ns_history_lookup_clear();ns_owned=0;check(!mprotect(dead,bytes,PROT_NONE));check(!ns_history_find((void*)0x1001,1));check(!munmap(dead,bytes));
 printf("{\"success\":true,\"checks\":%u}\n",checks);return 0;
}
