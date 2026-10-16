#include <stdint.h>
#include <string.h>
/* Explicit policy for the pinned software-driver stale-image resize case.
 * Geometry alone does not establish a universal GL image-origin convention. */
typedef struct {
 uint32_t old_width,old_height,width,height,window,generation;
 uint32_t shm_segment,shm_offset,total_width,total_height,full_gl_blit;
} MnmEglStaleImage;
static int mnm_egl_crop_stale_image(const MnmEglStaleImage* s,const uint32_t before[16],uint32_t after[16]){
 memcpy(after,before,16*sizeof(uint32_t));
 if(!s->full_gl_blit||!s->window||!s->generation||!s->shm_segment||!s->width||!s->height||s->width>s->old_width||s->height>=s->old_height||s->old_width>2048||s->old_height>2048||before[0]!=s->window||before[1]!=s->total_width||before[2]!=s->total_height||before[1]<s->old_width||before[2]<s->old_height||before[3]||before[4]||before[5]!=before[1]||before[6]!=before[2]||before[7]||before[8]||before[9]!=24||before[10]!=2||before[11]||before[12]||before[13]||before[14]!=s->shm_segment||before[15]!=s->shm_offset)return 0;
 after[4]=s->old_height-s->height;after[5]=s->width;after[6]=s->height;return 1;
}
