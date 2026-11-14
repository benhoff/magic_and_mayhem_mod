#include "fxsave-defined-bytes.h"
#include <string.h>
#include <stdio.h>
static int equal(const uint8_t* a,const uint8_t* b){for(unsigned i=0;i<560;++i)if(fixture_state_byte_defined(a,i)&&a[i]!=b[i])return 0;return 1;}
int main(void){
 uint8_t a[560]={0},b[560];unsigned controls=0;
 /* Actual ARM/Wine refusal: an empty ST2 slot acquired the1.0 payload. */
 a[51]=0x38;a[52]=0x80;memcpy(b,a,560);b[119]=0x80;b[120]=0xff;b[121]=0x3f;
 if(!equal(a,b))return 1;
 ++controls;
 /* The same physical payload is observable as soon as its tag becomes full. */
 a[52]|=2;memcpy(b,a,560);b[119]=0x80;if(equal(a,b))return 2;
 ++controls;
 for(unsigned offset=0;offset<560;++offset){
  if(offset==28||offset==32||offset==48||offset==50||offset==52||offset==54||offset==56||offset==64||offset==72||offset==80||offset==208||offset==335){
   memcpy(b,a,560);b[offset]^=1;if(equal(a,b))return 3;
   ++controls;
  }
 }
 memcpy(b,a,560);b[53]=1;b[62]=1;b[63]=1;b[70]=1;b[71]=1;b[90]=1;b[400]=1;
 if(!equal(a,b))return 4;
 ++controls;
 printf("%u controls: empty/reserved normalization; live x87, GPR/flags, control/status, tags, FIP/FDP, MXCSR and XMM changes refuse\n",controls);return controls==15?0:5;
}
