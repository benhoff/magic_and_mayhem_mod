#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
typedef uint32_t u32;typedef uint8_t u8;
#include "../runtime/native-draw/initial_history.h"
int main(void){
 u32 identity[8]={1,3,2,8,0,0,0,0};
 assert(mnm_owned_initial_identity(identity,3,2));
 for(unsigned i=0;i<8;++i){u32 saved=identity[i];identity[i]=i<4?0:1;
  assert(!mnm_owned_initial_identity(identity,3,2));identity[i]=saved;}
 identity[3]=7;assert(!mnm_owned_initial_identity(identity,3,2));identity[3]=8;
 assert(!mnm_owned_initial_identity(identity,2049,2));
 assert(!mnm_owned_initial_identity(identity,3,2049));
 assert(!mnm_owned_initial_identity(identity,0,2));
 assert(!mnm_owned_initial_identity(identity,3,0));
 identity[3]=4098;assert(!mnm_owned_initial_identity(identity,3,2));identity[3]=8;
 assert(!mnm_owned_initial_identity(identity,4,2));
 assert(!mnm_owned_initial_identity(identity,3,1));
 u8 bytes[16]={0};bytes[6]=bytes[7]=bytes[14]=bytes[15]=0xa5;
 assert(mnm_owned_initial_zero(bytes,3,2,8));
 for(unsigned y=0;y<2;++y)for(unsigned x=0;x<6;++x){bytes[y*8+x]=1;
  assert(!mnm_owned_initial_zero(bytes,3,2,8));bytes[y*8+x]=0;}
 /* Inclusive allocation limits and the last logical byte. */
 u8* maximum=calloc(2048,4096);assert(maximum);
 u32 largest[8]={1,2048,2048,4096,0,0,0,0};
 assert(mnm_owned_initial_identity(largest,2048,2048));
 assert(mnm_owned_initial_zero(maximum,2048,2048,4096));
 maximum[2048u*4096u-1]=1;
 assert(!mnm_owned_initial_zero(maximum,2048,2048,4096));free(maximum);
 puts("fresh native primary: identity/history/stride/padding/nonzero refusal controls passed");
}
