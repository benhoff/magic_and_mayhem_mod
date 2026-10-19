#ifndef MNM_WORLD_SOURCE_COPY_H
#define MNM_WORLD_SOURCE_COPY_H
/* Copy the current bytes on every capture. Permission caching never supplies
 * pixels/content. Shared with the producer-level source-epoch fixture. */
static void mnm_world_source_copy(void* destination,const void* source,unsigned bytes,unsigned fast){
 unsigned char* a=(unsigned char*)destination;const unsigned char* b=(const unsigned char*)source;
 if(fast){unsigned count=bytes/4;__asm__ __volatile__("cld; rep movsl" : "+D"(a), "+S"(b), "+c"(count) : : "memory","cc");bytes%=4;}
 while(bytes--)*a++=*b++;
}
#endif
