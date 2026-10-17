#ifndef MNM_NATIVE_GPU_COPY_CENSUS_V1_H
#define MNM_NATIVE_GPU_COPY_CENSUS_V1_H
#include <stdint.h>
/* Scalar diagnostic file, never rendering input or dereferenceable pointers. */
#define MNM_GPU_COPY_CENSUS_CAPACITY 8192u
#define MNM_GPU_COPY_CENSUS_WORDS 32u
#define MNM_GPU_COPY_CENSUS_BYTES (128u+128u*MNM_GPU_COPY_CENSUS_CAPACITY)
struct MnmGpuCopyCensus {uint32_t header[32];uint32_t rows[MNM_GPU_COPY_CENSUS_CAPACITY][32];};
#if defined(__cplusplus)
static_assert(sizeof(MnmGpuCopyCensus)==MNM_GPU_COPY_CENSUS_BYTES,"GPU copy census ABI");
#else
_Static_assert(sizeof(struct MnmGpuCopyCensus)==MNM_GPU_COPY_CENSUS_BYTES,"GPU copy census ABI");
#endif
/* Caller holds the existing copy/tracker guard. Publication follows row bytes. */
static void mnm_gpu_copy_census_record(struct MnmGpuCopyCensus* c,const uint32_t* row){
 uint32_t* h=c->header;
 if(h[8]==UINT32_MAX){h[7]=1;return;}++h[8];
 if(row[15]&2)++h[10];if(row[15]&1)++h[11];if(row[15]&4)++h[12];if(row[15]&8)++h[13];
 if(!row[16] && !row[18])++h[14];
 if(row[17]&0x80000000u)++h[15];if((row[17]&0x80000000u) && row[16])++h[16];
 if(row[18]&0x80000000u)++h[17];
 if(h[6]>=MNM_GPU_COPY_CENSUS_CAPACITY){++h[9];h[7]=1;return;}
 uint32_t* out=c->rows[h[6]];for(uint32_t i=0;i<32;++i)out[i]=row[i];out[0]=h[8];++h[6];
}
#endif
