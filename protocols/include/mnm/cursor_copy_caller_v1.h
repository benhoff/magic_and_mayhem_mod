#ifndef MNM_CURSOR_COPY_CALLER_V1_H
#define MNM_CURSOR_COPY_CALLER_V1_H
#include <stdint.h>
/* Private diagnostic snapshot, never a host pointer or persisted wire layout. */
struct MnmCursorCallerFrame {
 uint32_t index,caller,source,destination,slot,thread;
 uint32_t source_object,destination_object,source_generation,destination_generation;
};
static inline int mnm_cursor_caller_frame_matches(const struct MnmCursorCallerFrame* f,
 uint32_t index,uint32_t thread,uint32_t slot,uint32_t marker,uint32_t actual_return,uint32_t stack_destination,
 uint32_t source_object,uint32_t destination_object,uint32_t source_generation,uint32_t destination_generation){
 return f && index>=13 && index<=17 && f->index==index && f->caller && f->source && f->destination &&
  slot && f->slot==slot && thread && f->thread==thread && marker && actual_return==marker &&
  stack_destination==f->destination && source_object && destination_object && source_generation && destination_generation &&
  f->source_object==source_object && f->destination_object==destination_object &&
  f->source_generation==source_generation && f->destination_generation==destination_generation;
}
#endif
