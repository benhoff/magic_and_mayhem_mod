#pragma once
/* Bounded reconstruction of No-CD 0x58f890. No original image inputs. */
static int mnm_vertical_bar_valid(unsigned width,unsigned height,unsigned fill,unsigned format){
 return width>=2&&width<=128&&height>=2&&height<=128&&fill<=height-2&&format<=1;
}
static unsigned mnm_vertical_bar_pixel(unsigned width,unsigned height,unsigned fill,unsigned format,unsigned x,unsigned y){
 if(!mnm_vertical_bar_valid(width,height,fill,format)||x>=width||y>=height)return 0;
 if(!x||x==width-1||!y||y==height-1||y<height-1-fill)return 0;
 unsigned step=255/(fill<255?fill:255)+1;
 unsigned offset=y-(height-1-fill),value=offset*step>=255?0:255-offset*step;
 return format?(((value&0xf8)|0x1f00)<<2)&0xffff:(((value&0xfffc)|0xff00)<<3)&0xffff;
}
