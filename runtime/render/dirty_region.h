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
