/* Tight, owned native rows only. Include every storage byte, including unused
 * RGB32 bits: native equality is stronger than displayed RGB equality. */
static int command_dirty_region(const unsigned char* before,const unsigned char* after,
    unsigned width,unsigned height,unsigned bytes,unsigned region[4]){
    unsigned left=width,top=height,right=0,bottom=0,stride=width*bytes;
    for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){
        unsigned at=y*stride+x*bytes,b=0;
        while(b<bytes && before[at+b]==after[at+b])++b;
        if(b==bytes)continue;
        if(x<left)left=x;
        if(y<top)top=y;
        if(x+1>right)right=x+1;
        if(y+1>bottom)bottom=y+1;
    }
    if(!right)return 0;
    region[0]=left;region[1]=top;region[2]=right;region[3]=bottom;return 1;
}

/* A finite partition of the dirty envelope. Each cell has a tight rectangle;
 * choose it only when pixel bytes plus the 32-byte UPDATE envelope cost less
 * than the single rectangle. Dense input retains one UPDATE. No wire change. */
#define COMMAND_DIRTY_REGIONS 32u
static unsigned command_dirty_regions(const unsigned char* before,const unsigned char* after,
    unsigned width,unsigned height,unsigned bytes,unsigned regions[COMMAND_DIRTY_REGIONS][4]){
    unsigned whole[4];
    if(!command_dirty_region(before,after,width,height,bytes,whole))return 0;
    unsigned count=0,cost=0,stride=width*bytes;
    for(unsigned cy=0;cy<8;++cy)for(unsigned cx=0;cx<4;++cx){
        unsigned x0=whole[0]+(whole[2]-whole[0])*cx/4,x1=whole[0]+(whole[2]-whole[0])*(cx+1)/4;
        unsigned y0=whole[1]+(whole[3]-whole[1])*cy/8,y1=whole[1]+(whole[3]-whole[1])*(cy+1)/8;
        unsigned left=x1,top=y1,right=0,bottom=0;
        for(unsigned y=y0;y<y1;++y)for(unsigned x=x0;x<x1;++x){
            unsigned at=y*stride+x*bytes,b=0;
            while(b<bytes && before[at+b]==after[at+b])++b;
            if(b==bytes)continue;
            if(x<left)left=x;
            if(y<top)top=y;
            if(x+1>right)right=x+1;
            if(y+1>bottom)bottom=y+1;
        }
        if(right){
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
