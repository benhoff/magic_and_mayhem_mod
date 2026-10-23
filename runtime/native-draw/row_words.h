/* Fixed geometry under the caller's admitted synchronous lease. WORD order
 * remains forward even for overlapping views; row padding stays untouched. */
static void ns_copy_word_rows(u16* out,u32 out_stride,const u16* in,u32 in_stride,u32 width,u32 height){
 for(u32 y=0;y<height;++y){u16* target=out+y*out_stride;const u16* source=in+y*in_stride;
#if (defined(__i386__) || defined(__x86_64__)) && !defined(NS_COPY_FORCE_SCALAR)
  u32 count=width;
  /* REP retains individual forward WORD ordering, including overlapping rows.
   * No CRT, vector register or wider access is introduced. */
  __asm__ volatile("cld;rep movsw" : "+D"(target), "+S"(source), "+c"(count) : : "memory","cc");
#else
  for(u32 x=0;x<width;++x)target[x]=source[x];
#endif
 }
}
