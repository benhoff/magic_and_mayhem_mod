#include "../protocols/include/mnm/cursor_copy_state_v1.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
int main(void){
 struct MnmCursorCopyState* c=calloc(1,sizeof(*c));assert(c);
 assert(sizeof(*c)==MNM_CURSOR_COPY_STATE_BYTES);
 uint32_t row[80];for(uint32_t i=0;i<80;++i)row[i]=i+17;
 for(uint32_t i=0;i<MNM_CURSOR_COPY_STATE_CAPACITY;++i)mnm_cursor_copy_state_record(c,row);
 assert(c->header[6]==MNM_CURSOR_COPY_STATE_CAPACITY&&c->header[8]==MNM_CURSOR_COPY_STATE_CAPACITY&&!c->header[7]);
 for(uint32_t i=0;i<80;++i)assert(c->rows[0][i]==row[i]&&c->rows[MNM_CURSOR_COPY_STATE_CAPACITY-1][i]==row[i]);
 mnm_cursor_copy_state_record(c,row);assert(c->header[7]&&c->header[8]==MNM_CURSOR_COPY_STATE_CAPACITY+1&&c->header[6]==MNM_CURSOR_COPY_STATE_CAPACITY);
 c->header[8]=UINT32_MAX;mnm_cursor_copy_state_record(c,row);assert(c->header[8]==UINT32_MAX&&c->header[7]);
 free(c);puts("cursor companion bounded writer controls passed");return 0;
}
