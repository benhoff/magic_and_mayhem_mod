#include "colour_rect.h"
static uint32_t word(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8;}
static uint32_t channel(uint32_t s,uint32_t d,uint32_t mode){
    if(!mode)return s;
    if(mode==1)return (s>>1)+(s>>2)+(d>>2);
    if(mode==2)return (s>>1)+(d>>1);
    return (s>>2)+(d>>1)+(d>>2);
}
int mnm_colour_rect(const struct MnmPixelPlane *p,const struct MnmColourRect *r,uint32_t *last_blue){
    if(!mnm_pixel_plane(p)||!r||!last_blue||!r->width||r->width>2048||!r->height||r->height>2048||r->mode>3||
       !r->visible_width||!r->visible_height||r->source_x>r->width||r->visible_width>r->width-r->source_x||
       r->source_y>r->height||r->visible_height>r->height-r->source_y||r->x<0||r->y<0||
       (uint32_t)r->x>p->width||r->visible_width>p->width-(uint32_t)r->x||
       (uint32_t)r->y>p->height||r->visible_height>p->height-(uint32_t)r->y)return 0;
    const uint32_t green=p->format?31:63,shift=p->format?10:11;
    if(r->triples){
        const uintptr_t source=(uintptr_t)r->triples,dest=(uintptr_t)p->pixels;
        const uint32_t bytes=r->width*r->height*6,dbytes=p->stride*p->height*2;
        if(source>UINTPTR_MAX-bytes||dest>UINTPTR_MAX-dbytes||(source<dest+dbytes&&dest<source+bytes))return 0;
        for(uint32_t i=0;i<r->width*r->height;++i){const uint8_t *c=r->triples+i*6;
            if(word(c)>31||word(c+2)>green||word(c+4)>31)return 0;}
    }else if(r->red>31||r->green>green||r->blue>31)return 0;
    uint32_t blue=0;
    for(uint32_t y=0;y<r->visible_height;++y)for(uint32_t x=0;x<r->visible_width;++x){
        uint32_t red=r->red,g=r->green,b=r->blue;
        if(r->triples){const uint8_t *c=r->triples+((r->source_y+y)*r->width+r->source_x+x)*6;red=word(c);g=word(c+2);b=word(c+4);}
        uint16_t *at=p->pixels+(r->y+y)*p->stride+r->x+x;
        const uint32_t d=*at;blue=d&31;
        *at=(uint16_t)(channel(red,(d>>shift)&31,r->mode)<<shift|
                      channel(g,(d>>5)&green,r->mode)<<5|channel(b,d&31,r->mode));
    }
    *last_blue=blue;return 1;
}
