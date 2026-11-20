#ifndef MNM_GAMEPLAY_FPS_V1_H
#define MNM_GAMEPLAY_FPS_V1_H
#if __SIZEOF_INT__ != 4
#error Gameplay FPS requires 32-bit unsigned int
#endif
/* Little-endian 32-byte optional sidecar at <menu-channel>.gameplay.
 * Words: magic[2], version, bytes, seqlock, epoch, cumulative ticks, ready.
 * One original World owner writes; host reads. No pointers or draw counts.
 * Tick total wraps modulo 2^32. Epoch changes discard reset/discontinuous
 * samples. An absent/invalid sidecar never affects engine/menu admission. */
#define MNM_GAMEPLAY_FPS_BYTES 32u
#define MNM_GAMEPLAY_FPS_MAGIC0 0x474d4e4du
#define MNM_GAMEPLAY_FPS_MAGIC1 0x31535046u
static inline int mnm_gameplay_fps_header(const unsigned int* p){
    return p[0]==MNM_GAMEPLAY_FPS_MAGIC0 && p[1]==MNM_GAMEPLAY_FPS_MAGIC1 &&
        p[2]==1 && p[3]==MNM_GAMEPLAY_FPS_BYTES;
}
/* Publish only counter deltas from the original once-forwarded World call.
 * The recovered accepted-update path advances at most once per call. Unknown
 * jumps are discarded rather than counted as gameplay work. */
static inline void mnm_gameplay_fps_publish(unsigned int* p,unsigned int before,unsigned int after,
                                           unsigned int* previous,int* observed){
    unsigned int sequence=__atomic_load_n(p+4,__ATOMIC_RELAXED);
    __atomic_store_n(p+4,sequence+1,__ATOMIC_SEQ_CST);
    unsigned int delta=after-before;
    if((*observed && before!=*previous) || delta>1)
        __atomic_fetch_add(p+5,1,__ATOMIC_RELAXED);
    if(delta<=1)__atomic_fetch_add(p+6,delta,__ATOMIC_RELAXED);
    __atomic_store_n(p+7,1,__ATOMIC_RELAXED);
    *previous=after;*observed=1;
    __atomic_store_n(p+4,sequence+2,__ATOMIC_RELEASE);
}
#endif
