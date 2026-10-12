#pragma once
/* Caller-serialized diagnostic tokens only. No waits, handles or OS calls. */
typedef struct {unsigned token,owner,outer_identity,inner_identity;} MnmCursorGateToken;
typedef struct {unsigned next;MnmCursorGateToken cells[64];} MnmCursorGateTokens;
static MnmCursorGateToken* mnm_cursor_token_reserve(MnmCursorGateTokens* pool,
    unsigned owner,unsigned outer,unsigned inner) {
    if(!owner||!outer||!inner||outer==inner||pool->next==~0u)return 0;
    for(unsigned i=0;i<64;++i)if(!pool->cells[i].token){
        MnmCursorGateToken* cell=pool->cells+i;
        cell->token=++pool->next;cell->owner=owner;
        cell->outer_identity=outer;cell->inner_identity=inner;
        return cell;
    }
    return 0;
}
static MnmCursorGateToken* mnm_cursor_token_find(MnmCursorGateTokens* pool,
    unsigned owner,unsigned token) {
    if(!owner||!token)return 0;
    for(unsigned i=0;i<64;++i)if(pool->cells[i].token==token&&pool->cells[i].owner==owner)return pool->cells+i;
    return 0;
}
static int mnm_cursor_token_return(MnmCursorGateTokens* pool,unsigned owner,
    unsigned token,unsigned outer,unsigned inner) {
    MnmCursorGateToken* cell=mnm_cursor_token_find(pool,owner,token);
    if(!cell||cell->outer_identity!=outer||cell->inner_identity!=inner)return 0;
    cell->token=cell->owner=cell->outer_identity=cell->inner_identity=0;
    return 1;
}
