#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../protocols/include/mnm/cursor_copy_caller_v1.h"
static int matches(const struct MnmCursorCallerFrame* f){
 return mnm_cursor_caller_frame_matches(f,13,11,0x1000,0x2000,0x2000,0x4000,0x5000,0x6000,7,8);
}
int main(void){
 const struct MnmCursorCallerFrame valid={13,0x480100,0x3000,0x4000,0x1000,11,0x5000,0x6000,7,8};
 assert(matches(&valid));assert(!matches(0));unsigned refused=1;
 for(unsigned i=0;i<10;++i){struct MnmCursorCallerFrame f=valid;((uint32_t*)&f)[i]=0;assert(!matches(&f));++refused;}
 struct MnmCursorCallerFrame inner=valid;inner.slot=0x9000;
 assert(!matches(&inner));assert(matches(&valid));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,14,11,0x1000,0x2000,0x2000,0x4000,0x5000,0x6000,7,8));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,13,12,0x1000,0x2000,0x2000,0x4000,0x5000,0x6000,7,8));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,13,11,0x1000,0x2000,0x2001,0x4000,0x5000,0x6000,7,8));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,13,11,0x1000,0x2000,0x2000,0x4001,0x5000,0x6000,7,8));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,13,11,0x1000,0x2000,0x2000,0x4000,0x5001,0x6000,7,8));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,13,11,0x1000,0x2000,0x2000,0x4000,0x5000,0x6000,9,8));++refused;
 assert(!mnm_cursor_caller_frame_matches(&valid,13,11,0x1000,0x2000,0x2000,0x4000,0x5000,0x6000,7,9));++refused;
 printf("{\"success\":true,\"identity_refusals\":%u,\"nested_top_scope_and_restoration\":true,\"stale_slot_and_lifetime_generation_refusals\":true}\n",refused);
 return 0;
}
