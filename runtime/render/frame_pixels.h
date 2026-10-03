#pragma once
/* Included after the host's u8/u32/i32 typedefs. RGBA bytes, top row first. */
static u8 render_channel(u32 value,u32 mask){
    if(!mask)return 0;
    u32 shift=0;while(!(mask&1)){mask>>=1;++shift;}
    return (u8)((((value>>shift)&mask)*255U)/mask);
}
static int render_mask(u32 mask,u32 bits){
    if(!mask || (bits<32 && mask>>bits))return 0;
    while(!(mask&1))mask>>=1;
    return (mask&(mask+1))==0 && mask<=255;
}
static int render_pixels(u8* rgba,u32 width,u32 height,const u8* pixels,i32 pitch,
                         u32 bits,u32 red,u32 green,u32 blue,const u8* palette){
    if(!width||!height||width>2048||height>2048||!pixels)return 0;
    if(bits!=8 && bits!=16 && bits!=24 && bits!=32)return 0;
    const u32 bytes=bits/8;
    if(pitch==(-2147483647-1) || (u32)(pitch<0?-pitch:pitch)<width*bytes)return 0;
    if(bits==8){if(!palette)return 0;}
    else if(!render_mask(red,bits)||!render_mask(green,bits)||!render_mask(blue,bits)||
            (red&green)||(red&blue)||(green&blue))return 0;
    for(u32 y=0;y<height;++y){
        const u8* row=pixels+(i32)y*pitch;
        for(u32 x=0;x<width;++x){
            u8* out=rgba+(y*width+x)*4;
            if(bits==8){const u8* color=palette+row[x]*4;out[0]=color[0];out[1]=color[1];out[2]=color[2];}
            else {u32 value=0;for(u32 i=0;i<bytes;++i)value|=(u32)row[x*bytes+i]<<(i*8);
                out[0]=render_channel(value,red);out[1]=render_channel(value,green);out[2]=render_channel(value,blue);}
            out[3]=255;
        }
    }
    return 1;
}
