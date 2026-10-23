/* Canonical owned-format conversion and nearest-neighbour sampling. Native
 * policy: RGB expands to eight bits; indexed output uses the nearest palette
 * colour with the lowest index breaking a tie. Keys compare source raw values. */
static u32 ns_component(u32 value,u32 mask){
 if(!mask)return 0;while(!(mask&1)){mask>>=1;value>>=1;}
 return ((value&mask)*255+mask/2)/mask;
}
static u32 ns_pack_component(u32 value,u32 mask){
 if(!mask)return 0;u32 shift=0;while(!(mask&1)){mask>>=1;++shift;}
 return ((value*mask+127)/255)<<shift;
}
static u32 ns_convert(const struct NsState* d,const struct NsState* s,u32 value){
 if(s->bits==d->bits && s->masks[0]==d->masks[0] && s->masks[1]==d->masks[1] && s->masks[2]==d->masks[2])return value;
 u32 r,g,b;
 if(s->bits==8){u32 colour=s->palette->entries[value&255];r=colour&255;g=(colour>>8)&255;b=(colour>>16)&255;}
 else {r=ns_component(value,s->masks[0]);g=ns_component(value,s->masks[1]);b=ns_component(value,s->masks[2]);}
 if(d->bits!=8)return ns_pack_component(r,d->masks[0])|ns_pack_component(g,d->masks[1])|ns_pack_component(b,d->masks[2]);
 u32 nearest=0,distance=0xffffffffu;
 for(u32 i=0;i<d->palette->count;++i){u32 c=d->palette->entries[i];i32 dr=(i32)r-(i32)(c&255),dg=(i32)g-(i32)((c>>8)&255),db=(i32)b-(i32)((c>>16)&255);
  u32 current=(u32)(dr*dr+dg*dg+db*db);if(current<distance){distance=current;nearest=i;}}
 return nearest;
}
/* Avoid a freestanding PE32 compiler runtime division dependency. */
static u32 ns_sample_divide(u32 offset,u32 source,u32 target,u32* phase){
 if(!offset){if(phase)*phase=0;return 0;}
 if(source==target && target){if(phase)*phase=0;return offset;}
 if(source<=0xffffffffu/offset){u32 product=offset*source;if(phase)*phase=product%target;return product/target;}
 unsigned long long product=(unsigned long long)offset*source,remainder=0,result=0;
 for(u32 i=64;i;--i){remainder=(remainder<<1)|((product>>(i-1))&1u);result<<=1;
  if(remainder>=target){remainder-=target;result|=1;}}
 if(phase)*phase=(u32)remainder;return (u32)result;
}
static u32 ns_sample(u32 offset,u32 source,u32 target){return ns_sample_divide(offset,source,target,0);}
/* Exact floor((offset+n)*source/target), for admitted nonwrapping offsets.
 * Carry uses subtraction to avoid overflow when target is near UINT32_MAX.
 * The returned sample retains the existing modulo32 quotient convention. */
struct NsSampleCursor {u32 value,phase,step,residue,target;};
static struct NsSampleCursor ns_sample_begin(u32 offset,u32 source,u32 target){
 struct NsSampleCursor c;c.value=ns_sample_divide(offset,source,target,&c.phase);
 c.step=source/target;c.residue=source%target;c.target=target;return c;
}
static void ns_sample_next(struct NsSampleCursor* c){
 c->value+=c->step;
 if(c->phase>=c->target-c->residue){c->phase-=c->target-c->residue;++c->value;}
 else c->phase+=c->residue;
}
