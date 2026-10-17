#include "../protocols/include/mnm/native_copy_routes_v1.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(void){
 struct MnmCopyRoutes* c=calloc(1,sizeof(*c));assert(c);
 uint32_t row[40]={0};row[10]=4|32|128;row[11]=1;row[12]=1;row[15]=2;
 mnm_copy_routes_record(c,row);
 assert(c->header[6]==1 && c->header[8]==1 && c->header[10]==1 && c->header[17]==1 && c->header[18]==1 && c->header[19]==1 && c->header[20]==1);
 assert(c->rows[0][0]==1 && row[0]==0);
 mnm_copy_routes_skipped(c,1,1);assert(c->header[21]==1 && !c->header[7] && !c->header[9]);
 row[10]=row[11]=row[12]=0;
 for(uint32_t route=0;route<=3;++route){row[15]=route;mnm_copy_routes_record(c,row);}
 assert(c->header[10]==2 && c->header[11]==1 && c->header[12]==1 && c->header[13]==1);
 for(uint32_t i=5;i<MNM_COPY_ROUTES_CAPACITY;++i)mnm_copy_routes_record(c,row);
 assert(c->header[6]==MNM_COPY_ROUTES_CAPACITY && c->rows[MNM_COPY_ROUTES_CAPACITY-1][0]==MNM_COPY_ROUTES_CAPACITY);
 mnm_copy_routes_record(c,row);assert(c->header[7]==1 && c->header[8]==MNM_COPY_ROUTES_CAPACITY+1);
 c->header[8]=UINT32_MAX;mnm_copy_routes_record(c,row);assert(c->header[8]==UINT32_MAX && c->header[6]==MNM_COPY_ROUTES_CAPACITY);
 mnm_copy_routes_skipped(c,2,0);assert(c->header[9]==1 && c->header[22]==2);
 c->header[21]=UINT32_MAX;mnm_copy_routes_skipped(c,1,1);assert(c->header[21]==UINT32_MAX && c->header[22]==3);
 free(c);puts("{\"success\":true,\"capacity\":65536,\"overflow_and_exhaustion_rejected\":true}");return 0;
}
