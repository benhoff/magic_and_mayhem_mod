#include "../runtime/render/gpu_copy_commands.h"
#include <assert.h>
#include <stdio.h>
static unsigned calls,fail_at=999;
static uint32_t records[257][10];
static int emit(void* context,const uint32_t* fields){
 (void)context;unsigned at=calls++;if(at==fail_at)return 0;
 assert(at<257);for(unsigned i=0;i<10;++i)records[at][i]=fields[i];return 1;
}
int main(void){
 struct MnmGpuCopyPlan p={.version=1,.bytes=MNM_GPU_COPY_PLAN_BYTES,.count=2,.source_width=8,.source_height=6,.target_width=8,.target_height=6,.self=1};
 p.pieces[0]=(struct MnmGpuCopyPiece){0,0,4,2,1,0};p.pieces[1]=(struct MnmGpuCopyPiece){1,0,4,2,3,3};
 assert(mnm_gpu_copy_commands(&p,1,1,3,emit,0) && calls==3);
 assert(records[0][0]==1 && records[0][1]==3 && records[0][4]==8 && records[0][5]==6 && !records[0][8]);
 assert(records[1][0]==3 && records[2][0]==3 && records[1][1]==1 && records[2][1]==1);
 unsigned refusals=0;
 for(unsigned field=0;field<25;++field){
  struct MnmGpuCopyPlan q=p;uint32_t source=1,target=1,scratch=3;
  switch(field){
   case 0:q.version=2;break;case 1:q.bytes--;break;case 2:q.count=257;break;case 3:q.reserved=1;break;
   case 4:q.self=2;break;case 5:q.keyed=2;break;case 6:q.key=1;break;case 7:q.keyed=1;q.key=65536;break;
   case 8:source=0;break;case 9:target=0;break;case 10:target=2;break;case 11:scratch=0;break;case 12:scratch=1;break;
   case 13:q.source_width=0;break;case 14:q.source_height=2049;break;case 15:q.target_height=5;break;
   case 16:q.result=1;break;case 17:q.pieces[1].sx=8;break;case 18:q.pieces[1].width=0;break;
   case 19:q.pieces[1].height=7;break;case 20:q.pieces[1].dx=7;break;case 21:q.pieces[1].dy=6;break;
   case 22:q.pieces[1].width=UINT32_MAX;break;case 23:q.pieces[1].sy=UINT32_MAX;break;case 24:q.pieces[255].width=1;break;
  }
  calls=0;assert(!mnm_gpu_copy_commands(&q,source,target,scratch,emit,0) && calls==0);++refusals;
 }
 p.keyed=1;p.key=0x1234;calls=0;assert(mnm_gpu_copy_commands(&p,1,1,3,emit,0) && !records[0][8] && records[1][8]==1 && records[2][9]==0x1234);
 p.result=0x88760096;calls=0;assert(mnm_gpu_copy_commands(&p,1,1,3,emit,0) && calls==3);
 for(fail_at=0;fail_at<3;++fail_at){calls=0;assert(!mnm_gpu_copy_commands(&p,1,1,3,emit,0) && calls==fail_at+1);}fail_at=999;
 struct MnmGpuCopyPiece first=p.pieces[0],second=p.pieces[1];
 p.pieces[0]=(struct MnmGpuCopyPiece){0};p.pieces[1]=(struct MnmGpuCopyPiece){0};
 p.count=0;calls=0;assert(mnm_gpu_copy_commands(&p,1,1,0,emit,0) && !calls);
 p.pieces[0]=first;p.pieces[1]=second;p.self=0;p.count=2;calls=0;assert(mnm_gpu_copy_commands(&p,1,2,0,emit,0) && calls==2);
 assert(refusals==25);
 p.self=1;p.count=256;p.result=0;
 for(unsigned i=0;i<256;++i)p.pieces[i]=(struct MnmGpuCopyPiece){i%8,i%6,1,1,(i+1)%8,(i+1)%6};
 calls=0;assert(mnm_gpu_copy_commands(&p,1,1,3,emit,0)&&calls==257);
 for(unsigned i=0;i<256;++i){assert(records[i+1][0]==3&&records[i+1][1]==1);assert(records[i+1][2]==i%8&&records[i+1][3]==i%6);}
 const unsigned failures[]={0,1,128,256};
 for(unsigned i=0;i<4;++i){fail_at=failures[i];calls=0;assert(!mnm_gpu_copy_commands(&p,1,1,3,emit,0)&&calls==fail_at+1);}
 calls=0;assert(!mnm_gpu_copy_commands(&p,1,1,3,0,0)&&!calls);
 assert(!mnm_gpu_copy_commands(0,1,1,3,emit,0)&&!calls);
 puts("{\"success\":true,\"prepublication_refusals\":27,\"publication_failures\":7,\"maximum_clip_records\":257,\"source_frozen_once\":true}");return 0;
}
