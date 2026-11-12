#ifndef MNM_NATIVE_COPY_CLIP_REGIONS_V1_H
#define MNM_NATIVE_COPY_CLIP_REGIONS_V1_H
#include <stdint.h>
/* Stopped PE32 observation file. Numeric identities are launch-local tokens;
 * readers never dereference them. Each row includes the actual RGNDATA bytes
 * consumed by one existing provider query/copy, not inferred emitted pieces. */
#define MNM_COPY_CLIPS_VERSION 1u
#define MNM_COPY_CLIPS_CAPACITY 8192u
#define MNM_COPY_CLIPS_REGION_CAPACITY 256u
#define MNM_COPY_CLIPS_ROW_WORDS (64u+8u+4u*MNM_COPY_CLIPS_REGION_CAPACITY)
#define MNM_COPY_CLIPS_ROW_BYTES (4u*MNM_COPY_CLIPS_ROW_WORDS)
#define MNM_COPY_CLIPS_BYTES (128u+MNM_COPY_CLIPS_CAPACITY*MNM_COPY_CLIPS_ROW_BYTES)
struct MnmCopyClips {
 uint32_t header[32];uint32_t rows[MNM_COPY_CLIPS_CAPACITY][MNM_COPY_CLIPS_ROW_WORDS];
};
static void mnm_copy_clips_init(struct MnmCopyClips* out){
 for(unsigned i=0;i<32;++i)out->header[i]=0;
 const char magic[8]={'M','N','M','C','L','P','0','1'};
 for(unsigned i=0;i<8;++i)((uint8_t*)out->header)[i]=(uint8_t)magic[i];
 out->header[2]=1;out->header[3]=128;out->header[4]=MNM_COPY_CLIPS_ROW_BYTES;
 out->header[5]=MNM_COPY_CLIPS_CAPACITY;
}
static int mnm_copy_clips_record(struct MnmCopyClips* out,const uint32_t fields[64],const uint32_t* regions){
 uint32_t* h=out->header;
 if(h[10] || !fields || !regions || !fields[1] || fields[1]>6 || !fields[2] ||
    regions[0]!=32 || regions[1]!=1 || regions[2]>MNM_COPY_CLIPS_REGION_CAPACITY ||
    regions[3]!=regions[2]*16 || fields[14]!=regions[2] || fields[15]!=32+regions[2]*16){
  ++h[7];h[9]|=1;return 0;
 }
 if(h[6]>=MNM_COPY_CLIPS_CAPACITY){++h[7];h[9]|=2;return 0;}
 if(h[11] && h[11]!=fields[2]){++h[7];h[9]|=4;return 0;}
 h[11]=fields[2];uint32_t* row=out->rows[h[6]];
 for(unsigned i=0;i<MNM_COPY_CLIPS_ROW_WORDS;++i)((volatile uint32_t*)row)[i]=0;
 for(unsigned i=0;i<64;++i)row[i]=fields[i];
 row[0]=h[6]+1;
 for(unsigned i=0;i<8+4*regions[2];++i)row[64+i]=regions[i];
 ++h[6];h[8]=h[6];return 1;
}
static void mnm_copy_clips_close(struct MnmCopyClips* out){out->header[10]=1;}
#ifdef __cplusplus
static_assert(sizeof(MnmCopyClips)==MNM_COPY_CLIPS_BYTES,"Copy clip-region file ABI");
#else
_Static_assert(sizeof(struct MnmCopyClips)==MNM_COPY_CLIPS_BYTES,"Copy clip-region file ABI");
#endif
#endif
