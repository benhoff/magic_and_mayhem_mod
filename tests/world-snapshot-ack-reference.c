#include "../protocols/include/mnm/world_continuous_v5.h"
__declspec(dllimport) void __stdcall ExitProcess(unsigned);
static void check(int b){if(!b)ExitProcess(71);}
static void put(unsigned char* p,unsigned n,unsigned v){p+=4*n;for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(v>>(i*8));}
void start(void){
 unsigned char request[64]={0},ack[80],expected[80]={0},legacy[80];
 for(unsigned i=0;i<16;++i)put(request,i,0x11220100u+13*i);
 /* Independently authored byte contract, without production encoding helpers. */
 const char magic[8]={'M','N','M','S','A','C','K','1'};
 for(unsigned i=0;i<8;++i)expected[i]=(unsigned char)magic[i];
 put(expected,2,1);put(expected,3,80);
 for(unsigned i=4;i<=10;++i)put(expected,i,0x11220100u+13*i);
 put(expected,11,0x11220100u+13*12);put(expected,12,0x11220100u+13*14);put(expected,13,0x11220100u+13*15);
 put(expected,14,0x12345678);put(expected,15,7);put(expected,16,0x76543210);put(expected,17,9);put(expected,18,2);
 mnm_snapshot_ack_encode(ack,request,0x12345678,7,0x76543210,9);
 for(unsigned i=0;i<80;++i)check(ack[i]==expected[i]);
 check(mnm_snapshot_ack_valid(ack,80,request,0x12345678,7,0x76543210,9));
 for(unsigned i=0;i<80;++i){ack[i]^=1;check(!mnm_snapshot_ack_valid(ack,80,request,0x12345678,7,0x76543210,9));ack[i]^=1;}
 mnm_ack_encode(legacy,request,0x12345678,7,0x76543210,9);
 check(!mnm_snapshot_ack_valid(legacy,80,request,0x12345678,7,0x76543210,9));
 check(!mnm_ack_valid(ack,80,request,0x12345678,7,0x76543210,9));
 check(!mnm_snapshot_ack_valid(ack,79,request,0x12345678,7,0x76543210,9));
 check(!mnm_snapshot_ack_valid(ack,80,request,0,0,0x76543210,9));
 check(!mnm_snapshot_ack_valid(ack,80,request,0x12345678,7,0,0));
 request[48]^=1;check(!mnm_snapshot_ack_valid(ack,80,request,0x12345678,7,0x76543210,9));
 ExitProcess(0);
}
