/* Opt-in source reconstruction only. Original bitmap bytes are never read. */
#include "../../reconstruction/rendering/vertical_bar.h"
struct BarSource {u32 object,pixels,width,height,fill,format,generated,valid;};
static struct BarSource bar_sources[256];
static struct BarSource* bar_source(u32 object,int create){
 for(u32 i=0;i<256;++i)if(bar_sources[i].object==object&&object)return bar_sources+i;
 if(create&&object)for(u32 i=0;i<256;++i)if(!bar_sources[i].object){bar_sources[i].object=object;return bar_sources+i;}
 return 0;
}
static void bar_source_enter(struct Frame* f){
 struct BarSource* s=bar_source(f->ecx,1);if(!s){fail(99,f->tag);return;}
 s->valid=0;
 if(f->tag==55)return; /* Resize source is known only after original return. */
 if(!readable(f->ecx,48)){fail(99,f->tag);return;}
 f->bar[0]=get((void*)f->ecx);f->bar[1]=get((void*)(f->ecx+4));f->bar[2]=get((void*)(f->ecx+24));f->bar[3]=get((void*)0x6e1f88);f->bar[4]=get((void*)(f->ecx+28));
 if(!mnm_vertical_bar_valid(f->bar[0],f->bar[1],f->bar[2],f->bar[3])||!f->bar[4]){fail(99,f->tag);return;}
 f->bar[5]=1;
}
static void bar_source_return(const struct Frame* f){
 struct BarSource* s=bar_source(f->ecx,0);
 if(!s||!readable(f->ecx,48)){fail(99,f->tag);return;}
 u32 w=get((void*)f->ecx),h=get((void*)(f->ecx+4)),pixels=get((void*)(f->ecx+28));
 if(!mnm_vertical_bar_valid(w,h,0,0)||!pixels||get((void*)(f->ecx+36))!=w||get((void*)(f->ecx+40))!=h||get((void*)(f->ecx+44))!=1||get((void*)(f->ecx+32))!=w*h*2){fail(99,f->tag);return;}
 if(f->tag==55){
  if(w!=f->args[0]||h!=f->args[1]){fail(99,f->tag);return;}
  s->fill=s->format=s->generated=0;
 }else{
  if(!f->bar[5]||w!=f->bar[0]||h!=f->bar[1]||pixels!=f->bar[4]){fail(99,f->tag);return;}
  s->fill=f->bar[2];s->format=f->bar[3];s->generated=1;
 }
 s->pixels=pixels;s->width=w;s->height=h;s->valid=1;
}
static u8* bar_draw_request(const struct Frame* f){
 struct BarSource* src=bar_source(f->ecx,0);struct Surface* dst=object(f->args[1]);
 if(!src||!src->valid||!dst||!readable(f->ecx,48)||dst->pixels!=f->args[0]||!dst->pixels||get((void*)(f->ecx+28))!=src->pixels||get((void*)f->ecx)!=src->width||get((void*)(f->ecx+4))!=src->height||get((void*)0x6a49f4)!=dst->height||get((void*)0x6a49f8)!=dst->stride*2){fail(99,f->tag);return 0;}
 u32 rectangle=get((void*)0x6c482c);if(!readable(rectangle,16)){fail(99,f->tag);return 0;}
 i32 left=(i32)get((void*)rectangle),top=(i32)get((void*)(rectangle+4)),right=(i32)get((void*)(rectangle+8)),bottom=(i32)get((void*)(rectangle+12));
 i32 ox=(i32)get((void*)(f->ecx+8)),oy=(i32)get((void*)(f->ecx+12));
 if(left<0||top<0||right>(i32)dst->width||bottom>(i32)dst->height||left>right||top>bottom){fail(99,f->tag);return 0;}
 u8* out=HeapAlloc(producer_heap(),0,96+src->width*src->height*12);if(!out){fail(7,f->tag);return 0;}
 zero(out,96);u32* r=(u32*)out;r[2]=MNM_PRODUCER_POINTS;extent(r,dst);r[22]=f->caller;
 r[15]=src->generated?0x58f890:0x58f750;r[16]=src->fill;r[17]=src->format;r[19]=src->width;r[20]=src->height;
 u32 count=0;
 for(u32 y=0;y<src->height;++y)for(u32 x=0;x<src->width;++x){
  long long px=(long long)ox+x,py=(long long)oy+y;
  if(px<left||px>=right||py<top||py>=bottom)continue;
  u32 colour=src->generated?mnm_vertical_bar_pixel(src->width,src->height,src->fill,src->format,x,y):31;
  if(!point(out,&count,dst,(i32)px,(i32)py,colour)){HeapFree(producer_heap(),0,out);fail(99,f->tag);return 0;}
 }
 if(!count){HeapFree(producer_heap(),0,out);return 0;}
 r[14]=count;r[18]=count*12;r[0]=96+r[18];return out;
}
