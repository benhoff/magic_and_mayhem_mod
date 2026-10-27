/* Bounded read-only evidence for a refused canonical minimap row. The
 * admission rule and original/native drawing decisions remain unchanged. */
static u32 minimap_row_refusals;
static void minimap_row_refusal(const struct Frame* f,const struct Surface* dst,
 const u32* meta,u32 marker,i32 row,i32 delta,long long entry,long long expected){
 u32 serial=__sync_fetch_and_add(&minimap_row_refusals,1);if(serial>=4)return;
 u32 r[96];zero(r,sizeof(r));copy(r,"MNMMROW1",8);r[2]=1;r[3]=sizeof(r);r[4]=f->tag;r[5]=f->caller;r[6]=f->ecx;
 r[7]=dst->id;r[8]=dst->object;r[9]=dst->pixels;r[10]=dst->width;r[11]=dst->height;r[12]=dst->stride;
 r[13]=get((void*)(f->ecx+0xf3));r[14]=marker;r[15]=(u32)row;r[16]=(u32)delta;
 r[17]=(u32)entry;r[18]=(u32)((unsigned long long)entry>>32);r[19]=(u32)expected;r[20]=(u32)((unsigned long long)expected>>32);
 r[21]=entry>0&&entry<=0xffffffffll&&readable((u32)entry,4);if(r[21])r[22]=get((void*)(u32)entry);
 r[23]=GetCurrentThreadId();r[25]=queue;r[26]=sequence;
 r[28]=get((void*)(f->ecx+0x6199));r[29]=get((void*)(f->ecx+0x619d));r[30]=get((void*)(f->ecx+0xeb));
 if(r[21])for(u32 i=0;i<MAX_SURFACES;++i){const struct Surface* other=surfaces+i;
  if(other->id&&other->pixels&&r[22]>=other->pixels&&(unsigned long long)r[22]<(unsigned long long)other->pixels+(unsigned long long)other->stride*other->height*2){
   ++r[31];r[32]=other->id;r[33]=other->pixels;r[34]=other->stride;r[35]=other->width;r[36]=other->height;r[37]=other->object;}}
 copy(r+48,meta,37*4);
 char name[260];path(name,"producer-minimap-row-",serial,".bin");HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){write(file,r,sizeof(r));CloseHandle(file);}
}
