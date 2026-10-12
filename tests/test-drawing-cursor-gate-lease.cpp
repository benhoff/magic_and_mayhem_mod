#include "../runtime/cursor-mutex/cursor_gate_lease.h"
#include <cassert>
#include <cstdint>
static void* h(unsigned n){return reinterpret_cast<void*>(uintptr_t(n));}
int main(){
 MnmCursorGateTokens pool{};MnmCursorLifetime outer{},inner{};
 assert(mnm_cursor_lifetime_publish(&outer,h(1),h(2),1));
 assert(mnm_cursor_lifetime_publish(&inner,h(3),h(4),2));
 auto* token=mnm_cursor_lease_reserve(&pool,&outer,&inner,10,h(1),h(3));assert(token);
 unsigned value=token->token;assert(outer.leases==1&&inner.leases==1);
 assert(!mnm_cursor_lease_return(&pool,&outer,&inner,11,value,0));
 assert(!mnm_cursor_lease_return(&pool,&outer,&inner,10,value,1));
 assert(outer.leases==1&&inner.leases==1);
 assert(mnm_cursor_lifetime_retire(&inner,h(3),2));
 assert(!mnm_cursor_lease_reserve(&pool,&outer,&inner,11,h(1),h(3)));
 assert(outer.leases==1&&inner.leases==1);
 assert(!mnm_cursor_lifetime_close_begin(&inner));
 assert(mnm_cursor_lease_return(&pool,&outer,&inner,10,value,0));
 assert(!outer.leases&&!inner.leases);
 assert(!mnm_cursor_lease_return(&pool,&outer,&inner,10,value,0));
 assert(mnm_cursor_lifetime_close_begin(&inner)==h(4));
 assert(mnm_cursor_lifetime_close_end(&inner,h(4),2,1));
 assert(mnm_cursor_lifetime_publish(&inner,h(3),h(5),3));
 token=mnm_cursor_lease_reserve(&pool,&outer,&inner,10,h(1),h(3));assert(token&&token->token!=value);
 assert(!mnm_cursor_lease_return(&pool,&outer,&inner,10,value,0));
 assert(mnm_cursor_lease_return(&pool,&outer,&inner,10,token->token,0));
 assert(!mnm_cursor_lease_reserve(&pool,&outer,&inner,10,h(7),h(3)));
 assert(!outer.leases&&!inner.leases);
 assert(!mnm_cursor_lease_reserve(&pool,&outer,&outer,10,h(1),h(1)));
 // Pool exhaustion must not add untracked lifetime reservations.
 unsigned full[64];for(unsigned i=0;i<64;++i){auto* held=mnm_cursor_lease_reserve(&pool,&outer,&inner,i+1,h(1),h(3));assert(held);full[i]=held->token;}
 assert(outer.leases==64&&inner.leases==64);
 assert(!mnm_cursor_lease_reserve(&pool,&outer,&inner,100,h(1),h(3)));
 assert(outer.leases==64&&inner.leases==64);
 for(unsigned i=0;i<64;++i)assert(mnm_cursor_lease_return(&pool,&outer,&inner,i+1,full[i],0));
 assert(!outer.leases&&!inner.leases);
}
