#pragma once
/* Prospective caller-serialized owning-module core. No installation/live use.
 * Lock callbacks are caller obligations; all kernel calls occur outside them. */
#include "diagnostic_lifetime.h"
#include "cursor_gate_lease.h"
#include "../drawing/cursor_source_gate.h"
typedef struct {MnmCursorGateToken* token;MnmCursorSourceGate gate;} MnmCursorKernelLease;
typedef struct {
 MnmCursorLifetime outer,inner;
 MnmCursorGateTokens tokens;
 MnmCursorKernelLease leases[64];
 unsigned fault;
} MnmCursorKernelAdapter;
/* Reserve under caller lock, then acquire outside lock. Numeric token binds
 * actual gate storage, not a requester-supplied acquired count. */
static MnmCursorKernelLease* mnm_cursor_kernel_reserve(MnmCursorKernelAdapter* adapter,
 unsigned owner,void* outer,void* inner){
 if(adapter->fault)return 0;
 MnmCursorGateToken* token=mnm_cursor_lease_reserve(&adapter->tokens,&adapter->outer,&adapter->inner,owner,outer,inner);
 if(!token)return 0;
 unsigned slot=(unsigned)(token-adapter->tokens.cells);
 MnmCursorKernelLease* lease=adapter->leases+slot;
 if(lease->token||lease->gate.acquired){
  adapter->fault=1;
  /* A model/token mismatch means prior state is uncertain. Retain this newly
   * reserved identity/ref in the token table, freeze admission and report fault. */
  return 0;
 }
 lease->token=token;lease->gate.outer=adapter->outer.duplicate;
 lease->gate.inner=adapter->inner.duplicate;lease->gate.failure=0;
 return lease;
}
static int mnm_cursor_kernel_return(MnmCursorKernelAdapter* adapter,unsigned owner,unsigned token){
 MnmCursorGateToken* identity=mnm_cursor_token_find(&adapter->tokens,owner,token);
 if(!identity)return 0;
 unsigned slot=(unsigned)(identity-adapter->tokens.cells);
 MnmCursorKernelLease* lease=adapter->leases+slot;
 if(lease->token!=identity||lease->gate.acquired)return 0;
 if(!mnm_cursor_lease_return(&adapter->tokens,&adapter->outer,&adapter->inner,owner,token,lease->gate.acquired))return 0;
 lease->token=0;lease->gate.outer=lease->gate.inner=0;lease->gate.failure=0;
 return 1;
}
