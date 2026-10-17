#include "../runtime/cursor-mutex/cursor_gate_tokens.h"
#include <cassert>
int main(){
 MnmCursorGateTokens pool{};
 unsigned tokens[64];
 for(unsigned i=0;i<64;++i){auto* cell=mnm_cursor_token_reserve(&pool,i+1,1,2);assert(cell);tokens[i]=cell->token;assert(tokens[i]==i+1);}
 assert(!mnm_cursor_token_reserve(&pool,65,1,2));
 for(unsigned i=0;i<64;++i){
  assert(!mnm_cursor_token_find(&pool,100,tokens[i]));
  assert(!mnm_cursor_token_return(&pool,i+1,tokens[i],2,1));
  assert(mnm_cursor_token_find(&pool,i+1,tokens[i]));
  assert(mnm_cursor_token_return(&pool,i+1,tokens[i],1,2));
  assert(!mnm_cursor_token_return(&pool,i+1,tokens[i],1,2));
 }
 auto* cell=mnm_cursor_token_reserve(&pool,1,3,4);assert(cell&&cell->token==65);
 assert(!mnm_cursor_token_find(&pool,1,tokens[0]));
 assert(!mnm_cursor_token_reserve(&pool,0,1,2));
 assert(!mnm_cursor_token_reserve(&pool,1,0,2));
 assert(!mnm_cursor_token_reserve(&pool,1,1,0));
 assert(!mnm_cursor_token_reserve(&pool,1,1,1));
 pool.next=~0u-1;cell=mnm_cursor_token_reserve(&pool,2,1,2);assert(cell&&cell->token==~0u);
 assert(!mnm_cursor_token_reserve(&pool,3,1,2));
 assert(mnm_cursor_token_return(&pool,2,~0u,1,2));
 assert(!mnm_cursor_token_reserve(&pool,3,1,2));
}
