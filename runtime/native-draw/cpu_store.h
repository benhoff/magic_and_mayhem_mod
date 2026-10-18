/* PE32 flat-address scalar stores. No instruction executes with stale public
 * pages writable. The caller validates the complete decoded span before writing
 * its private alias. REP, LOCK, address/segment prefixes and RMW are rejected. */
struct NsCpuStore {u32 address,value,width,length,stos;};
static u32 ns_cpu_u32(const u8* p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}
static u32 ns_cpu_register(const u32* c,u32 reg){
 static const u8 offsets[8]={44,43,42,41,49,45,40,39};return c[offsets[reg&7]];
}
static int ns_cpu_decode_store(const u8* pc,const u32* c,struct NsCpuStore* s){
 const u8* p=pc;u32 width=4;if(*p==0x66){width=2;++p;}
 u32 op=*p++,reg=0,immediate=0,address=0;
 s->stos=0;
 if(op==0xaa || op==0xab){
  s->address=c[39];s->value=c[44];s->width=op==0xaa?1:width;s->length=(u32)(p-pc);s->stos=1;return 1;
 }
 if(op==0xa2 || op==0xa3){
  s->address=ns_cpu_u32(p);p+=4;s->value=c[44];s->width=op==0xa2?1:width;s->length=(u32)(p-pc);return 1;
 }
 if(op!=0x88 && op!=0x89 && op!=0xc6 && op!=0xc7)return 0;
 if(op==0x88 || op==0xc6)width=1;
 u32 modrm=*p++,mod=modrm>>6,rm=modrm&7;reg=(modrm>>3)&7;
 if(mod==3 || ((op==0xc6 || op==0xc7) && reg))return 0;
 if(rm==4){
  u32 sib=*p++,base=sib&7,index=(sib>>3)&7;
  if(index!=4)address+=ns_cpu_register(c,index)<<(sib>>6);
  if(mod==0 && base==5){address+=ns_cpu_u32(p);p+=4;}else address+=ns_cpu_register(c,base);
 }else if(mod==0 && rm==5){address=ns_cpu_u32(p);p+=4;}
 else address=ns_cpu_register(c,rm);
 if(mod==1){address+=(u32)(i32)(signed char)*p++;}
 else if(mod==2){address+=ns_cpu_u32(p);p+=4;}
 if(op==0xc6 || op==0xc7){
  immediate=1;s->value=*p++;
  if(width>=2){s->value|=(u32)*p++<<8;}
  if(width==4){s->value|=(u32)*p++<<16;s->value|=(u32)*p++<<24;}
 }
 if(!immediate){
  s->value=ns_cpu_register(c,width==1?reg&3:reg);
  if(width==1 && reg>=4)s->value>>=8;
 }
 s->address=address;s->width=width;s->length=(u32)(p-pc);return 1;
}
