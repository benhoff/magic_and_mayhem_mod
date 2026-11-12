#ifndef MNM_CURSOR_COPY_STATE_V1_H
#define MNM_CURSOR_COPY_STATE_V1_H
#include <stdint.h>
/* Diagnostic scalar companion to the closed v2 COPY census, not an input.
 * Words0..43 are an exact original COPY row. See the versioned format note. */
#define MNM_CURSOR_COPY_STATE_CAPACITY 8192u
#define MNM_CURSOR_COPY_STATE_WORDS 80u
#define MNM_CURSOR_COPY_STATE_BYTES (128u+320u*MNM_CURSOR_COPY_STATE_CAPACITY)
struct MnmCursorCopyState {uint32_t header[32];uint32_t rows[MNM_CURSOR_COPY_STATE_CAPACITY][80];};
static void mnm_cursor_copy_state_record(struct MnmCursorCopyState* c,const uint32_t* row){
 uint32_t* h=c->header;
 if(h[8]==UINT32_MAX){h[7]=1;return;}++h[8];
 if(h[6]>=MNM_CURSOR_COPY_STATE_CAPACITY){h[7]=1;return;}
 for(uint32_t i=0;i<80;++i)c->rows[h[6]][i]=row[i];
 ++h[6];
}
#endif
