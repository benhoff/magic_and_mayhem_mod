#pragma once
/* Caller serializes all model operations. No OS waits/closes. */
#include "diagnostic_lifetime.h"
#include "cursor_gate_tokens.h"
static MnmCursorGateToken* mnm_cursor_lease_reserve(MnmCursorGateTokens* pool,
    MnmCursorLifetime* outer,MnmCursorLifetime* inner,unsigned owner,
    void* original_outer,void* original_inner) {
    if(outer==inner)return 0;
    MnmCursorGateToken* token=mnm_cursor_token_reserve(pool,owner,outer->identity,inner->identity);
    if(!token)return 0;
    if(!mnm_cursor_lifetime_reserve(outer,original_outer,token->outer_identity)) {
        mnm_cursor_token_return(pool,owner,token->token,token->outer_identity,token->inner_identity);
        return 0;
    }
    if(!mnm_cursor_lifetime_reserve(inner,original_inner,token->inner_identity)) {
        mnm_cursor_lifetime_return(outer,token->outer_identity);
        mnm_cursor_token_return(pool,owner,token->token,token->outer_identity,token->inner_identity);
        return 0;
    }
    return token;
}
static int mnm_cursor_lease_return(MnmCursorGateTokens* pool,
    MnmCursorLifetime* outer,MnmCursorLifetime* inner,unsigned owner,unsigned value,unsigned acquired_levels) {
    MnmCursorGateToken* token=mnm_cursor_token_find(pool,owner,value);
    if(acquired_levels||!token||outer==inner||outer->identity!=token->outer_identity||
        inner->identity!=token->inner_identity||!outer->leases||!inner->leases)return 0;
    unsigned a=token->outer_identity,b=token->inner_identity;
    mnm_cursor_lifetime_return(inner,b);mnm_cursor_lifetime_return(outer,a);
    return mnm_cursor_token_return(pool,owner,value,a,b);
}
