/* CPU execution for 2D effects outside the narrow GPU COPY wire contract.
 * Inputs are owned surfaces; publication uses the existing owned CPU snapshot.
 * ROP3 evaluates its complete eight-entry (pattern, source, destination) table. */
static u32 ns_rop3(u32 table,u32 pattern,u32 source,u32 destination){
 u32 result=0;
 for(u32 i=0;i<8;++i)if(table&(1u<<i))
  result|=(i&4?pattern:~pattern)&(i&2?source:~source)&(i&1?destination:~destination);
 return result;
}
static struct NsState* ns_pattern(u32 object){
 for(struct NsState* s=ns_owned;s;s=s->next_owned)for(u32 i=0;i<5;++i)
  if(object==(u32)&s->aliases[i])return s;
 return 0;
}
static int ns_effect_palette(const struct NsState* d,const struct NsState* s){
 return !s || (s->bits==d->bits && s->masks[0]==d->masks[0] && s->masks[1]==d->masks[1] && s->masks[2]==d->masks[2]) ||
  ((s->bits!=8 || s->palette) && (d->bits!=8 || d->palette));
}
static i32 ns_effect_draw(struct NsObject* dst,const i32* dr,struct NsObject* src,const i32* sr,u32 flags,const u32* fx){
 struct NsState *d=dst->state,*s=src?src->state:0,*pattern=0;
 const u32 supported=0x01000000u|0x08000000u|0x200u|0x400u|0x800u|0x1e000u|0x20000u|0x40000u;
 if(flags&~supported)return NS_UNSUPPORTED;
 const u32 needsFx=flags&(0x400u|0x800u|0x14000u|0x20000u|0x40000u);
 if((needsFx && (!fx || fx[0]!=100)) || (!needsFx && fx))return ND_INVALID;
 const u32 fill=flags&0x400u,rop=flags&0x20000u?(fx[2]>>16)&255u:0xccu;
 const u32 needsSource=((rop^(rop>>2))&0x33u)!=0 && !fill;
 if((needsSource && !s) || (fill && (s || (flags&0x20000u))))return ND_INVALID;
 if((flags&0x40000u) && fx[4]%9000u)return NS_UNSUPPORTED;
 u32 effect=flags&0x800u?fx[1]:0;
 if(effect&~0x7eu)return NS_UNSUPPORTED;
 u32 rotation=effect&0x70u;
 if(rotation && rotation!=0x10 && rotation!=0x20 && rotation!=0x40)return ND_INVALID;
 if(flags&0x40000u){if(rotation)return ND_INVALID;u32 quarter=(fx[4]/9000u)%4;
  rotation=quarter==1?0x40:quarter==2?0x10:quarter==3?0x20:0;}
 if((rop^(rop>>4))&15u){pattern=ns_pattern(fx[20]);if(!pattern)return ND_INVALID;}
 if(!ns_effect_palette(d,s) || !ns_effect_palette(d,pattern))return ND_INVALID;
 u32 sourceKey=flags&0x18000u,destKey=flags&0x6000u;
 if(sourceKey==0x18000u || destKey==0x6000u || (sourceKey && !s))return ND_INVALID;
 u32 sl=0,sh=0,dl=0,dh=0;
 if(sourceKey){if(sourceKey==0x10000u){sl=fx[23];sh=fx[24];}else {if(!s->key_set)return ND_INVALID;sl=s->key_low;sh=s->key_high;}}
 if(destKey){if(destKey==0x4000u){dl=fx[21];dh=fx[22];}else {if(!d->dest_key_set)return ND_INVALID;dl=d->dest_key_low;dh=d->dest_key_high;}}
 if(sl>sh || dl>dh)return ND_INVALID;
 i32 rd[4]={0,0,(i32)d->width,(i32)d->height},rs[4]={0,0,s?(i32)s->width:0,s?(i32)s->height:0};
 if(dr)for(u32 i=0;i<4;++i)rd[i]=dr[i];if(sr)for(u32 i=0;i<4;++i)rs[i]=sr[i];
 long long width=(long long)rd[2]-rd[0],height=(long long)rd[3]-rd[1];
 if(width<=0 || height<=0 || width>0xffffffffu || height>0xffffffffu ||
    (s && !ns_rectangle(s,rs,rs)) || (!d->clipper && !ns_rectangle(d,rd,rd)))return NS_RECT;
 if(ns_borrowed(d) || (s && ns_borrowed(s)) || (pattern && ns_borrowed(pattern)))return NS_BUSY;
 u32* clips=0;i32 result=0;if(d->clipper){result=nc_snapshot(d->clipper,&clips);if(result)return result;}
 ns_cpu_require(d);if(s)ns_cpu_require(s);if(pattern)ns_cpu_require(pattern);
 /* A single frozen alias serves both source and pattern for self operations. */
 u8* frozen=0;struct NsState snapshot;
 if(s==d || pattern==d){frozen=HeapAlloc(GetProcessHeap(),0,d->pitch*d->height);
  if(!frozen){result=(i32)0x8007000eu;goto done;}
  const u8* bytes=(const u8*)(d->pixels_alias?d->pixels_alias:d->pixels);
  for(u32 i=0;i<d->pitch*d->height;++i)frozen[i]=bytes[i];
  snapshot.bits=d->bits;snapshot.width=d->width;snapshot.height=d->height;snapshot.pitch=d->pitch;snapshot.palette=d->palette;
  for(u32 i=0;i<3;++i)snapshot.masks[i]=d->masks[i];snapshot.pixels=(u16*)frozen;snapshot.pixels_alias=0;
  if(s==d)s=&snapshot;if(pattern==d)pattern=&snapshot;
 }
 if(!d->pixels_alias)ns_guard_set(d,0);
 for(u32 region=0;region<(clips?clips[2]:1u);++region){
  const i32* clip=clips?(const i32*)(clips+8+4*region):0;
  i32 left=rd[0]<0?0:rd[0],top=rd[1]<0?0:rd[1];
  i32 right=rd[2]>(i32)d->width?(i32)d->width:rd[2],bottom=rd[3]>(i32)d->height?(i32)d->height:rd[3];
  if(clip){if(left<clip[0])left=clip[0];if(top<clip[1])top=clip[1];if(right>clip[2])right=clip[2];if(bottom>clip[3])bottom=clip[3];}
  for(i32 y=top;y<bottom;++y)for(i32 x=left;x<right;++x){
   u32 value=0,old=ns_read(d,x,y),p=0;
   if(destKey && (old<dl || old>dh))continue;
   if(s){u32 sw=(u32)(rs[2]-rs[0]),shh=(u32)(rs[3]-rs[1]),dx=(u32)((long long)x-rd[0]),dy=(u32)((long long)y-rd[1]);
    u32 sx=ns_sample(dx,sw,(u32)width),sy=ns_sample(dy,shh,(u32)height);
    if(rotation==0x10){sx=sw-1-sx;sy=shh-1-sy;}
    else if(rotation==0x40){sx=ns_sample(dy,sw,(u32)height);sy=shh-1-ns_sample(dx,shh,(u32)width);}
    else if(rotation==0x20){sx=sw-1-ns_sample(dy,sw,(u32)height);sy=ns_sample(dx,shh,(u32)width);}
    if(effect&2)sx=sw-1-sx;if(effect&4)sy=shh-1-sy;
    value=ns_read(s,(u32)rs[0]+sx,(u32)rs[1]+sy);
    if(sourceKey && value>=sl && value<=sh)continue;
    value=ns_convert(d,s,value);
   }
   if(pattern)p=ns_convert(d,pattern,ns_read(pattern,(u32)x%pattern->width,(u32)y%pattern->height));
   ns_write(d,x,y,fill?fx[20]:ns_rop3(rop,p,value,old));
  }
 }
 ++ns_copies;
done:if(frozen)HeapFree(GetProcessHeap(),0,frozen);if(clips)HeapFree(GetProcessHeap(),0,clips);ns_guard_refresh(d);return result;
}
