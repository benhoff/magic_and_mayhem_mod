#ifndef MNM_GENERIC_PRODUCER_CALLER_H
#define MNM_GENERIC_PRODUCER_CALLER_H
/* Pinned cdecl continuations consume AX after restoring 20 argument bytes.
 * Flags are overwritten by ADD/CMP; other incoming state is retained policy.
 * Callers outside this selected domain are not inferred from another bypass. */
static int mnm_generic_producer_caller(unsigned caller,unsigned tag,const unsigned char* bytes){
 static const unsigned addresses[]={0x500512,0x500a4e,0x57e940};
 static const unsigned char signatures[][10]={
  {0x83,0xc4,0x14,0x66,0x3d,0x01,0x00,0x0f,0x85,0x29},
  {0x83,0xc4,0x14,0x66,0x3d,0x01,0x00,0x0f,0x85,0x77},
  {0x83,0xc4,0x14,0x66,0x3d,0x01,0x00,0x75,0x11,0x8b}};
 if((tag!=13&&tag!=14)||!bytes)return 0;
 for(unsigned i=0;i<3;++i)if(caller==addresses[i]){
  for(unsigned j=0;j<10;++j)if(bytes[j]!=signatures[i][j])return 0;
  return 1;
 }
 return 0;
}
#endif
