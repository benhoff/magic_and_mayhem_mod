/* All loads remain unaligned-safe. Native equality includes every storage
 * byte and every nonzero dirty flag, including same-value pending writes. */
static unsigned command_dirty_word(const unsigned char* p){
 return (unsigned)p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;
}
static unsigned command_dirty_half(const unsigned char* p){return (unsigned)p[0]|(unsigned)p[1]<<8;}
static int command_clean16(const unsigned char* before,const unsigned char* after,const unsigned char* mask){
 unsigned changed=0;
 for(unsigned i=0;i<16;i+=4){changed|=command_dirty_word(before+i)^command_dirty_word(after+i);if(mask)changed|=command_dirty_word(mask+i);}
 return changed==0;
}
static int command_dirty_pixel(const unsigned char* a,const unsigned char* b,const unsigned char* mask,unsigned bytes){
 if(bytes==2)return ((command_dirty_half(a)^command_dirty_half(b))|(mask?command_dirty_half(mask):0))!=0;
 if(bytes==4)return ((command_dirty_word(a)^command_dirty_word(b))|(mask?command_dirty_word(mask):0))!=0;
 for(unsigned i=0;i<bytes;++i)if(a[i]!=b[i]||(mask&&mask[i]))return 1;
 return 0;
}
/* A nonclean block is visited once, then each of its complete pixels. This
 * avoids a sliding16-byte rescan for every pixel in dense updates. */
static int command_dirty_scan(const unsigned char* before,const unsigned char* after,unsigned stride,unsigned bytes,
 const unsigned char* mask,unsigned x0,unsigned y0,unsigned x1,unsigned y1,unsigned region[4]){
 unsigned left=x1,top=y1,right=0,bottom=0,block=bytes==1?16:bytes==2?8:bytes==4?4:0;
 for(unsigned y=y0;y<y1;++y)for(unsigned x=x0;x<x1;){
  unsigned at=y*stride+x*bytes,step=block&&block<=x1-x?block:1;
  if(block&&step==block&&command_clean16(before+at,after+at,mask?mask+at:0)){x+=step;continue;}
  unsigned end=x+step;
  for(;x<end;++x){at=y*stride+x*bytes;
   if(!command_dirty_pixel(before+at,after+at,mask?mask+at:0,bytes))continue;
   if(x<left)left=x;
   if(y<top)top=y;
   if(x+1>right)right=x+1;
   if(y+1>bottom)bottom=y+1;
  }
 }
 if(!right)return 0;
 region[0]=left;region[1]=top;region[2]=right;region[3]=bottom;return 1;
}
static int command_dirty_region_mask(const unsigned char* before,const unsigned char* after,
 unsigned width,unsigned height,unsigned bytes,const unsigned char* mask,unsigned region[4]){
 return command_dirty_scan(before,after,width*bytes,bytes,mask,0,0,width,height,region);
}
static int command_dirty_region(const unsigned char* before,const unsigned char* after,
 unsigned width,unsigned height,unsigned bytes,unsigned region[4]){
 return command_dirty_region_mask(before,after,width,height,bytes,0,region);
}

/* A finite partition of the dirty envelope. Each cell has a tight rectangle;
 * choose it only when pixel bytes plus the 32-byte UPDATE envelope cost less
 * than the single rectangle. Dense input retains one UPDATE. No wire change. */
#define COMMAND_DIRTY_REGIONS 32u
static unsigned command_dirty_regions_mask(const unsigned char* before,const unsigned char* after,
    unsigned width,unsigned height,unsigned bytes,const unsigned char* mask,unsigned regions[COMMAND_DIRTY_REGIONS][4]){
    unsigned whole[4];
    if(!(mask?command_dirty_region_mask(before,after,width,height,bytes,mask,whole):command_dirty_region(before,after,width,height,bytes,whole)))return 0;
    unsigned count=0,cost=0,stride=width*bytes;
    for(unsigned cy=0;cy<8;++cy)for(unsigned cx=0;cx<4;++cx){
        unsigned x0=whole[0]+(whole[2]-whole[0])*cx/4,x1=whole[0]+(whole[2]-whole[0])*(cx+1)/4;
        unsigned y0=whole[1]+(whole[3]-whole[1])*cy/8,y1=whole[1]+(whole[3]-whole[1])*(cy+1)/8;
        unsigned part[4];
        if(command_dirty_scan(before,after,stride,bytes,mask,x0,y0,x1,y1,part)){
            unsigned left=part[0],top=part[1],right=part[2],bottom=part[3];
            regions[count][0]=left;regions[count][1]=top;regions[count][2]=right;regions[count++][3]=bottom;
            cost+=(right-left)*(bottom-top)*bytes+32;
        }
    }
    if(cost>=(whole[2]-whole[0])*(whole[3]-whole[1])*bytes+32){
        for(unsigned i=0;i<4;++i)regions[0][i]=whole[i];
        return 1;
    }
    return count;
}
static unsigned command_dirty_regions(const unsigned char* before,const unsigned char* after,
    unsigned width,unsigned height,unsigned bytes,unsigned regions[COMMAND_DIRTY_REGIONS][4]){
    return command_dirty_regions_mask(before,after,width,height,bytes,0,regions);
}
