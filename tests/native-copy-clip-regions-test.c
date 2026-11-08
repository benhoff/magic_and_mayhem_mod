#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../protocols/include/mnm/native_copy_clip_regions_v1.h"
int main(int argc,char** argv){
 struct MnmCopyClips* out=calloc(1,sizeof(*out));assert(out);mnm_copy_clips_init(out);
 uint32_t fields[64]={0,1,7,0x1000,0x2000,0x1100,0x2100,1,2,0x3000,0x4000,1,0,0,2,64,
  0,0,4,3,0,0,4,3,4,3,16,4,3,16,0,0,2};
 uint32_t regions[16]={32,1,2,32,0,0,4,3,0,0,2,3,2,0,4,3};
 assert(mnm_copy_clips_record(out,fields,regions));assert(out->header[6]==1 && out->header[8]==1);
 fields[16]=123;regions[8]=99;assert(out->rows[0][16]==0 && out->rows[0][72]==0);
 mnm_copy_clips_close(out);
 if(argc==2){FILE* f=fopen(argv[1],"wb");assert(f);assert(fwrite(out,sizeof(*out),1,f)==1);assert(!fclose(f));}
 assert(!mnm_copy_clips_record(out,fields,regions));assert(out->header[6]==1 && out->header[7]==1);
 out->header[10]=0;fields[16]=0;regions[8]=0;regions[2]=257;
 assert(!mnm_copy_clips_record(out,fields,regions));assert(out->header[6]==1);
 regions[2]=2;fields[2]=8;assert(!mnm_copy_clips_record(out,fields,regions));
 assert(out->header[9]&4);fields[2]=7;
 out->header[6]=MNM_COPY_CLIPS_CAPACITY;out->header[8]=MNM_COPY_CLIPS_CAPACITY;
 assert(!mnm_copy_clips_record(out,fields,regions));assert(out->header[9]&2);
 free(out);puts("native clip-region recorder: immutable input, closure, malformed count, thread and capacity guards passed");return 0;
}
