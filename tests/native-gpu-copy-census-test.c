#include "../protocols/include/mnm/native_gpu_copy_census_v1.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
int main(void){
 struct MnmGpuCopyCensus* c=calloc(1,sizeof(*c));assert(c);
 uint32_t row[32]={0};row[1]=0x401234;row[15]=15;row[16]=2;row[17]=0x88760096;
 mnm_gpu_copy_census_record(c,row);assert(c->header[6]==1 && c->header[8]==1 && c->header[10]==1 && c->header[11]==1 && c->header[12]==1 && c->header[13]==1 && c->header[15]==1 && c->header[16]==1);
 assert(c->rows[0][0]==1 && c->rows[0][1]==0x401234 && row[0]==0);
 row[15]=0;row[16]=row[17]=0;mnm_gpu_copy_census_record(c,row);assert(c->header[14]==1);
 row[17]=row[18]=0x80000000;mnm_gpu_copy_census_record(c,row);assert(c->header[17]==1 && c->header[14]==1);
 for(unsigned i=3;i<MNM_GPU_COPY_CENSUS_CAPACITY;++i)mnm_gpu_copy_census_record(c,row);
 assert(c->header[6]==8192 && c->rows[8191][0]==8192);
 mnm_gpu_copy_census_record(c,row);assert(c->header[6]==8192 && c->header[8]==8193 && c->header[9]==1 && c->header[7]==1);
 c->header[8]=UINT32_MAX;mnm_gpu_copy_census_record(c,row);assert(c->header[8]==UINT32_MAX && c->header[9]==1);
 free(c);puts("{\"success\":true,\"capacity\":8192,\"overflow_and_counter_exhaustion_refused\":true}");return 0;
}
