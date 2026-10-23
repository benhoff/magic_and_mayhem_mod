/* Private dirty-mask bookkeeping. The caller owns geometry, mask and counters
 * under its existing synchronous lease. Mark bytes in their original order. */
static void ns_dirty_linear(u8* dirty,u32 bit,u32 bytes,u32* count,u32* partial){
 for(u32 i=0;i<bytes;++i,++bit){u32 index=bit>>3,flag=1u<<(bit&7);
  if(dirty[index]&flag)continue;
  if(dirty[index]&(1u<<((bit^1u)&7)))--*partial;else ++*partial;
  dirty[index]|=(u8)flag;++*count;
 }
}
static void ns_dirty_bytes(u8* dirty,u32 width,u32 height,u32 pitch,u32 offset,u32 bytes,u32* count,u32* partial){
 if(!bytes)return;
 const u32 row=offset/pitch,col=offset%pitch,row_bytes=width*2;
 if(row<height && col<row_bytes && bytes<=row_bytes-col){
  ns_dirty_linear(dirty,row*row_bytes+col,bytes,count,partial);return;
 }
 /* Padding, row crossings and unsigned wrap retain the complete scalar rule. */
 for(u32 i=0;i<bytes;++i){u32 at=offset+i,y=at/pitch,x=at%pitch;
  if(y<height && x<row_bytes)ns_dirty_linear(dirty,y*row_bytes+x,1,count,partial);
 }
}

#include <stdint.h>
/* Expand a private bitmap into the existing one-byte-per-image-byte contract.
 * Disjoint buffers permit one lookup per nibble; aliasing retains the original
 * ordered scalar reads and writes. There is no allocation or rounded access. */
static void ns_expand_dirty_mask(u8* out,const u8* bits,u32 bytes){
 if(!bytes)return;
 const uintptr_t dst=(uintptr_t)out,src=(uintptr_t)bits;
 const u32 input_bytes=bytes/8+(bytes%8!=0);
 if(dst>UINTPTR_MAX-bytes || src>UINTPTR_MAX-input_bytes ||
    (dst<src+input_bytes && src<dst+bytes)){
  for(u32 i=0;i<bytes;++i)out[i]=(bits[i>>3]>>(i&7))&1;
  return;
 }
 static const u32 spread[16]={0,1,0x100,0x101,0x10000,0x10001,0x10100,0x10101,
  0x1000000,0x1000001,0x1000100,0x1000101,0x1010000,0x1010001,0x1010100,0x1010101};
 u32 i=0;
 for(;bytes-i>=8;i+=8){
  const u32 mask=bits[i>>3],lo=spread[mask&15],hi=spread[mask>>4];
  out[i]=(u8)lo;out[i+1]=(u8)(lo>>8);out[i+2]=(u8)(lo>>16);out[i+3]=(u8)(lo>>24);
  out[i+4]=(u8)hi;out[i+5]=(u8)(hi>>8);out[i+6]=(u8)(hi>>16);out[i+7]=(u8)(hi>>24);
 }
 for(;i<bytes;++i)out[i]=(bits[i>>3]>>(i&7))&1;
}
