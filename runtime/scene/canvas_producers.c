/* Bounded closed producer inputs. Original destination bytes are oracle files. */
#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/canvas_producers_v2.h"
#include "../../protocols/include/mnm/canvas_producers_v3.h"
#define HOOKS 52
#define MAX_SURFACES 128
#define MAX_FRAMES 128
#define E(n) extern void producer_enter_##n(void)
E(0);E(1);E(2);E(3);E(4);E(5);E(6);E(7);E(8);E(9);E(10);E(11);E(12);E(13);E(14);E(15);E(16);E(17);E(18);E(19);E(20);E(21);E(22);E(23);E(24);E(25);E(26);E(27);E(28);E(29);E(30);E(31);E(32);E(33);E(34);E(35);E(36);E(37);E(38);E(39);E(40);E(41);E(42);E(43);E(44);E(45);E(46);E(47);E(48);E(49);E(50);E(51);
extern void producer_leave(void);
u32 producer_trampolines[HOOKS];
/* pop is the original RET immediate, independent of compiler calling labels. */
static struct Hook {u32 address,length,pop;u8 bytes[10];void(*entry)(void);i32 raster;} hooks[HOOKS]={
 {0x58ad90,5,12,{0xa1,0x54,0x81,0x6f,0},producer_enter_0,-1},
 {0x58b3e0,5,0,{0x53,0x56,0x8b,0xf1,0x57},producer_enter_1,-1},
 {0x58b660,5,0,{0x83,0xec,8,0x33,0xc0},producer_enter_2,-1},
 {0x57dc20,10,4,{0x8b,0x44,0x24,4,0x89,0x0d,0x74,0x81,0x65,0},producer_enter_3,-1},
 {0x57dc60,10,8,{0x8b,0x44,0x24,4,0x89,0x0d,0x6c,0xbb,0x6c,0},producer_enter_4,-1},
 {0x58bc10,6,4,{0x83,0xec,0x6c,0x56,0x8b,0xf1},producer_enter_5,-1},
 {0x58bac0,6,8,{0x83,0xec,0x6c,0x56,0x8b,0xf1},producer_enter_6,-1},
 {0x58bf40,5,12,{0x83,0xec,0x30,0x53,0x55},producer_enter_7,-1},
 {0x58c140,5,12,{0x83,0xec,0x30,0x53,0x55},producer_enter_8,-1},
 {0x58d320,9,4,{0x56,0x57,0x8b,0xf1,0xe8,0x37,0xe3,0xff,0xff},producer_enter_9,-1},
 {0x581ec0,7,16,{0x83,0xec,0x28,0x8b,0x44,0x24,0x30},producer_enter_10,-1},
 {0x57e1b0,5,12,{0x83,0xec,0x14,0x53,0x55},producer_enter_11,1},
 {0x595677,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_12,0},
 {0x5947b2,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_13,1},
 {0x59521a,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_14,1},
 {0x595b47,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_15,2},
 {0x59603e,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_16,2},
 {0x57de00,6,0,{0x55,0x8b,0xec,0x83,0xec,0x18},producer_enter_17,3},
 {0x57ec90,6,12,{0x83,0xec,0x1c,0x53,0x55,0x56},producer_enter_18,4},
 {0x57f0f0,6,12,{0x83,0xec,0x24,0x53,0x55,0x56},producer_enter_19,5},
 {0x57f5f0,6,12,{0x83,0xec,0x24,0x53,0x55,0x56},producer_enter_20,6},
 {0x5806f0,7,8,{0x83,0xec,0x20,0x8b,0x44,0x24,0x28},producer_enter_21,7},
 {0x597086,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_22,8},
 {0x596490,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_23,9},
 {0x5968a4,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_24,9},
 {0x57e540,8,4,{0x83,0xec,0x18,0xa1,0x88,0x1f,0x6e,0},producer_enter_25,10},
 {0x545c10,8,8,{0x83,0xec,0x1c,0xa1,0x88,0x1f,0x6e,0},producer_enter_26,-1},
 {0x58ddf0,8,16,{0x83,0xec,0x40,0xa1,0x88,0x1f,0x6e,0},producer_enter_27,-1},
 {0x58d8c0,8,4,{0x83,0xec,0x38,0xa1,0x88,0x1f,0x6e,0},producer_enter_28,-1},
 {0x58e2c0,8,4,{0x83,0xec,0x24,0xa1,0x88,0x1f,0x6e,0},producer_enter_29,-1},
 {0x54b2d0,5,4,{0x83,0xec,0x18,0x53,0x57},producer_enter_30,-1},
 {0x58d240,7,4,{0x51,0x56,0x8b,0xf1,0x8b,0x46,0x08},producer_enter_31,-1},
 {0x58c360,6,16,{0x83,0xec,0x18,0x33,0xd2,0x53},producer_enter_32,-1},
 {0x58c4a0,6,16,{0x83,0xec,0x20,0x33,0xd2,0x53},producer_enter_33,-1},
 {0x589c90,8,0,{0x83,0xec,0x1c,0xa1,0x88,0x1f,0x6e,0},producer_enter_34,11},
 {0x58d280,7,12,{0x6a,0xff,0x68,0x88,0x2e,0x5c,0},producer_enter_35,-1},
 {0x596cb8,6,0,{0x55,0x8b,0xec,0x56,0x57,0x53},producer_enter_36,8},
 {0x58c6a0,8,16,{0x83,0xec,0x20,0xa1,0xd8,0x68,0x6f,0},producer_enter_37,-1},
 {0x58c8a0,7,16,{0x83,0xec,0x20,0x8b,0x44,0x24,0x2c},producer_enter_38,-1},
 {0x58ca90,8,16,{0x83,0xec,0x18,0x53,0x8b,0x5c,0x24,0x28},producer_enter_39,-1},
 {0x58d1a0,7,4,{0x6a,0xff,0x68,0x68,0x2e,0x5c,0},producer_enter_40,-1},
 {0x58cbc0,8,12,{0x83,0xec,0x10,0xa1,0xd8,0x68,0x6f,0},producer_enter_41,-1},
 {0x58cd50,8,16,{0x83,0xec,0x20,0x53,0x8b,0x5c,0x24,0x2c},producer_enter_42,-1},
 {0x58cf20,8,16,{0x83,0xec,0x20,0x53,0x8b,0x5c,0x24,0x2c},producer_enter_43,-1},
 {0x58ed80,9,0,{0x51,0x55,0x56,0x8b,0x35,0x88,0x1f,0x6e,0},producer_enter_44,-1},
 {0x553a40,6,0,{0x83,0xec,0x20,0x56,0x8b,0xf1},producer_enter_45,-1},
 {0x5527a0,7,0,{0x83,0xec,0x28,0x53,0x55,0x56,0x57},producer_enter_46,-1},
 {0x5536c0,7,16,{0x53,0x8b,0x1d,0x94,0x54,0x6c,0},producer_enter_47,-1},
 {0x553850,5,0,{0x83,0xec,8,0x53,0x55},producer_enter_48,-1},
 {0x552b50,6,0,{0x83,0xec,0x28,0x53,0x55,0x56},producer_enter_49,-1},
 {0x552f20,6,0,{0x83,0xec,0x28,0x53,0x55,0x56},producer_enter_50,-1},
 {0x5532f0,6,0,{0x83,0xec,0x28,0x53,0x55,0x56},producer_enter_51,-1}
};
static struct Surface {u32 object,id,pixels,width,height,stride,dirty,sampled;} surfaces[MAX_SURFACES];
static struct Frame {u32 sp,tag,caller,ecx,edx,args[6],primitive;u8* record;} frames[MAX_FRAMES];
static HANDLE file;
/* Batch observation owns a separate heap; canvas guard pages include the
 * original allocation metadata and must never be relaxed for observer buffers. */
static HANDLE producer_record_heap;
static HANDLE producer_heap(void){return producer_record_heap?producer_record_heap:GetProcessHeap();}
static char directory[220];
static u32 enabled,stopped,sequence,bytes,next_id,oracle,oracle_bytes,queue,limit,depth,failures;
static u32 minimap_owned,minimap_pace;
static u32 oracle_byte_limit=1024u*1024u*1024u;
/* Diagnostic storage policy, not an original engine limit. Stay within u32. */
static int oracle_configure(const char* number,u32 n){
 if(!n||n>4)return 0;u32 mib=0;
 for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return 0;mib=mib*10+number[i]-'0';}
 if(!mib||mib>3072)return 0;oracle_byte_limit=mib*1024u*1024u;return 1;
}
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static void zero(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
static int write(HANDLE f,const void* p,u32 n){u32 at=0;while(at<n){u32 done=0;if(!WriteFile(f,(const u8*)p+at,n-at,&done,0)||!done||done>n-at)return 0;at+=done;}return 1;}
static void path(char* out,const char* name,u32 number,const char* suffix){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;for(u32 d=1000;d;d/=10)out[i++]=(char)('0'+number/d%10);while(*suffix)out[i++]=*suffix++;out[i]=0;}
void canvas_producers_close(void){if(file){CloseHandle(file);file=0;}stopped=1;}
static void emit_raw(u32* r,const void* payload,u32 n){if(!file||stopped)return;if(sequence>=(limit>16?MNM_PRODUCER_V3_MAX_RECORDS:minimap_owned?MNM_PRODUCER_V2_MAX_RECORDS:MNM_PRODUCER_MAX_RECORDS)||bytes>(minimap_owned?MNM_PRODUCER_V2_MAX_BYTES:MNM_PRODUCER_MAX_BYTES)-96||n>(minimap_owned?MNM_PRODUCER_V2_MAX_BYTES:MNM_PRODUCER_MAX_BYTES)-bytes-96){canvas_producers_close();return;}r[0]=96+n;r[1]=++sequence;r[18]=n;if(!write(file,r,96)||(n&&!write(file,payload,n))){canvas_producers_close();return;}bytes+=96+n;}
/* V3 source snapshots live on the private producer heap. No original pointers
 * or destination pixels enter this cache; equality includes every state byte. */
static struct SourcePacket {u32 sequence,kind,tag,size,hash;u8* data;} source_packets[MNM_PRODUCER_V3_SOURCES];
static u32 source_packet_count,source_packet_bytes;
static void emit(u32* r,const void* payload,u32 n){
 if(limit<=16||(r[2]!=8&&r[2]!=9)||!n){emit_raw(r,payload,n);return;}
 const u8* data=payload;u32 hash=2166136261u;for(u32 i=0;i<n;++i)hash=(hash^data[i])*16777619u;
 struct SourcePacket* source=0;
 for(u32 i=0;i<source_packet_count;++i){struct SourcePacket* s=source_packets+i;if(s->kind!=r[2]||s->tag!=r[17]||s->size!=n||s->hash!=hash)continue;u32 j=0;while(j<n&&s->data[j]==data[j])++j;if(j==n){source=s;break;}}
 if(!source&&source_packet_count<MNM_PRODUCER_V3_SOURCES&&n<=MNM_PRODUCER_V3_SOURCE_BYTES-source_packet_bytes){
  u8* owned=HeapAlloc(producer_heap(),0,n);
  if(owned){copy(owned,data,n);u32 definition[24]={0};definition[2]=MNM_PRODUCER_PAYLOAD_SOURCE;definition[14]=r[2];definition[17]=r[17];definition[19]=r[19];definition[20]=r[20];emit_raw(definition,owned,n);
   source=source_packets+source_packet_count++;source->sequence=definition[1];source->kind=r[2];source->tag=r[17];source->size=n;source->hash=hash;source->data=owned;source_packet_bytes+=n;}
 }
 if(!source){emit_raw(r,payload,n);return;} /* Cache exhaustion keeps raw input. */
 u32 wire[24];copy(wire,r,96);wire[4]=source->sequence;emit_raw(wire,0,0);
 /* Keep the producer's full canonical packet for exact batch descriptor hashes. */
 r[0]=96+n;r[1]=wire[1];r[18]=n;
}
static void fail(u32 why,u32 detail){++failures;u32 r[24]={0};r[2]=MNM_PRODUCER_FAILURE;r[14]=why;r[15]=detail;emit(r,0,0);}
static struct Surface* object(u32 p){for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].object==p&&p)return surfaces+i;return 0;}
static struct Surface* bound(void){u32 p=get((void*)0x658174);for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].object&&surfaces[i].pixels==p&&p)return surfaces+i;return 0;}
static void extent(u32* r,struct Surface* s){if(s){r[3]=s->id;r[5]=s->width;r[6]=s->height;r[7]=s->stride;}}
static void clip(u32* r){r[10]=get((void*)0x6e0008);r[11]=get((void*)0x6cbb6c);r[12]=get((void*)0x6a49b8);r[13]=get((void*)0x656618);}
static void checkpoint(struct Surface* s,u32 reason){
 if(!s||!s->pixels||!s->dirty||s->dirty==s->sampled)return;
 if(!s->width||s->width>2048||!s->height||s->height>2048||s->stride<s->width||s->stride>4096||!readable(s->pixels,s->stride*s->height*2)){fail(10,s->id);return;}
 u32 n=s->width*s->height*2;if(oracle>=9999||oracle_bytes>oracle_byte_limit||n>oracle_byte_limit-oracle_bytes){fail(11,s->id);return;}
 char name[260];path(name,"producer-oracle-",++oracle,".565");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(f==(HANDLE)-1){fail(12,s->id);return;}
 int okay=1;for(u32 y=0;y<s->height;++y)if(!write(f,(void*)(s->pixels+y*s->stride*2),s->width*2)){okay=0;break;}if(!CloseHandle(f))okay=0;
 if(!okay){fail(13,s->id);return;}oracle_bytes+=n;s->sampled=s->dirty;
 u32 r[24]={0};r[2]=MNM_PRODUCER_CHECKPOINT;extent(r,s);r[14]=oracle;r[15]=reason;r[16]=queue;emit(r,0,0);
}
static struct Surface* map_destination(u32 p){u32 base=get((void*)(p+0xdf));for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].object&&surfaces[i].pixels==base)return surfaces+i;return 0;}
static int point(u8* out,u32* count,struct Surface* dst,i32 x,i32 y,u32 colour){if(*count>=16384)return 0;long long offset=(long long)y*dst->stride+x;if(offset<0||offset>=(long long)dst->stride*dst->height||(u32)offset%dst->stride>=dst->width)return 0;u8* p=out+96+(*count)++*12;put(p,(u32)offset%dst->stride);put(p+4,(u32)offset/dst->stride);put(p+8,colour&0xffff);return 1;}
static i32 wrap(i32 x,u32 size){i32 n=x%(i32)size;return n<0?n+(i32)size:n;}
static u8* map_points(const struct Frame* f){
 u32 p=f->ecx,tag=f->tag;if(!readable(p,0x6208)){fail(32,tag);return 0;}struct Surface* dst=map_destination(p);u32 w=get((void*)0x6c5494),h=get((void*)0x6c5498);if(!dst||!w||w>256||!h||h>256||get((void*)(p+0xc5))||get((void*)0x6e1f88)){fail(33,tag);return 0;}
 u8* out=HeapAlloc(producer_heap(),0,96+16384*12);if(!out){fail(7,tag);return 0;}zero(out,96);u32* r=(u32*)out;r[2]=MNM_PRODUCER_POINTS;extent(r,dst);r[22]=f->caller;u32 count=0;int okay=1;i32 ox=(i32)get((void*)(p+0xf7)),oy=(i32)get((void*)(p+0xfb)),cx=(i32)get((void*)(p+0x61a1)),cy=(i32)get((void*)(p+0x61a5));
 if(tag==46){u32 rect=get((void*)0x6c482c);if(!readable(rect,16))okay=0;else{ i32 a=(i32)get((void*)(p+0x103))/2,b=(i32)get((void*)(p+0x107))/2,sw=((i32)get((void*)(rect+8))-(i32)get((void*)rect))/128+1,sh=((i32)get((void*)(rect+12))-(i32)get((void*)(rect+4)))/64;for(u32 corner=0;corner<4;++corner){i32 vx=(i32)get((void*)(p+0x6121+corner*8)),vy=(i32)get((void*)(p+0x6125+corner*8));i32 x=(a+b)/2-vx*sw*2,y=(a+b)/2-1-vy*sh+(vy<0);for(u32 n=0;n<4;++n){okay&=point(out,&count,dst,ox+x-b+(i32)n*vx,oy+y,0x400);okay&=point(out,&count,dst,ox+x-b,oy+y+(i32)n*vy,0x400);}}}}
 else if(tag==47){u32 index=f->args[2];if(index>1024||!readable(p+0xcb+index*2,2))okay=0;else{i32 u=wrap((i32)f->args[0]+(i32)(w/2)-cx,w),v=wrap((i32)f->args[1]+(i32)(h/2)-cy,h);u32 colour=*(u16*)(p+0xcb+index*2);i32 x=ox+u-v,y=oy+(u+v)/2;if(f->args[3]&&get((void*)(p+0x6141))){const i32 dx[8]={-1,2,1,0,0,1,2,-1},dy[8]={-1,-1,0,0,1,1,2,2};for(u32 k=0;k<8;++k)okay&=point(out,&count,dst,x+dx[k],y+dy[k],colour);}else{const i32 dx[4]={0,1,0,1},dy[4]={0,0,1,1};for(u32 k=0;k<4;++k)okay&=point(out,&count,dst,x+dx[k],y+dy[k],colour);}}}
 else{u32 begin=get((void*)(p+0x614d)),end=get((void*)(p+0x6151));if(end<begin||(end-begin)%12||(end-begin)/12>1024||(!readable(begin,end-begin)&&begin!=end))okay=0;else for(u32 n=begin;n<end;n+=12){i32 rawx=(i32)get((void*)n),rawy=(i32)get((void*)(n+4));u32 z=get((void*)(n+8));if(rawy<0||rawy>=128||z/2>=32){okay=0;break;}u32 state=get((void*)0x6c5c5c)+get((void*)(0x6cb942+(u32)rawy*4))+get((void*)(0x6cb8c2+(z/2)*4))+(u32)rawx;if(!readable(state,1)){okay=0;break;}if((i32)(signed char)*(u8*)state==(i32)get((void*)0x5e18f0)&&get((void*)0x5e1404))continue;i32 u=wrap(rawx+(i32)(w/2)-cx,w),v=wrap(rawy+(i32)(h/2)-cy,h),x=ox+u-v,y=oy+(u+v)/2;const i32 dx[13]={-1,0,1,2,-2,-2,0,2,2,-2,-1,0,1},dy[13]={-2,-2,-2,-1,-1,0,0,0,1,1,2,2,2};for(u32 k=0;k<13;++k)okay&=point(out,&count,dst,x+dx[k],y+dy[k],0xfc00);}}
 if(!okay){HeapFree(producer_heap(),0,out);fail(34,tag);return 0;}r[14]=count;r[18]=count*12;r[0]=96+r[18];return out;
}
static u8* owned_map_overlay(const struct Frame* f){
 u32 p=f->ecx,tag=f->tag;if(!readable(p,0x620b)){fail(32,tag);return 0;}
 struct Surface* dst=map_destination(p);u32 w=get((void*)0x6c5494),h=get((void*)0x6c5498);
 if(!dst||!w||w>256||!h||h>256||get((void*)(p+0xc5))>3||get((void*)0x6e1f88)>1){fail(33,tag);return 0;}
 u32 meta[37]={0};meta[0]=tag==47?0:tag==48?1:2;meta[1]=w;meta[2]=h;
 meta[3]=get((void*)(p+0x61a1));meta[4]=get((void*)(p+0x61a5));meta[5]=get((void*)(p+0x103));meta[6]=get((void*)(p+0x107));
 if(meta[3]>=w||meta[4]>=h||meta[5]>256||meta[6]>256){fail(34,tag);return 0;}
 meta[7]=get((void*)(p+0xf7));meta[8]=get((void*)(p+0xfb));meta[9]=get((void*)(p+0xc5));meta[10]=get((void*)0x6e1f88);meta[11]=get((void*)0x5e1404)!=0;meta[12]=get((void*)(p+0x6141))!=0;
 u32 begin=0,end=0;
 if(meta[0]==0){for(u32 i=0;i<4;++i)meta[13+i]=f->args[i];if(meta[15]>=9){fail(34,tag);return 0;}for(u32 i=0;i<9;++i)meta[28+i]=*(u16*)(p+0xcb+i*2);}
 else if(meta[0]==1){begin=get((void*)(p+0x614d));end=get((void*)(p+0x6151));if(end<begin||(end-begin)%12||(end-begin)/12>1024||(begin!=end&&!readable(begin,end-begin))){fail(34,tag);return 0;}meta[17]=(end-begin)/12;
 }else{u32 rect=get((void*)0x6c482c);if(!readable(rect,16)||!get((void*)(p+0x6207))){fail(37,tag);return 0;}meta[18]=get((void*)(rect+8))-get((void*)rect);meta[19]=get((void*)(rect+12))-get((void*)(rect+4));for(u32 i=0;i<8;++i)meta[20+i]=get((void*)(p+0x6121+i*4));}
 u32 n=148+meta[17]*12;u8* out=HeapAlloc(producer_heap(),0,96+n);if(!out){fail(7,tag);return 0;}zero(out,96);u32* r=(u32*)out;r[0]=96+n;r[2]=MNM_PRODUCER_OWNED_MINIMAP;r[17]=meta[0]==2?(tag==46?0:tag==49?1:tag==50?2:3):0;r[18]=n;r[22]=f->caller;extent(r,dst);copy(out+96,meta,148);
 for(u32 i=0;i<meta[17];++i){u32 x=get((void*)(begin+i*12)),y=get((void*)(begin+i*12+4)),z=get((void*)(begin+i*12+8));if(x>=w||y>=h||y>=128||z/2>=32){HeapFree(producer_heap(),0,out);fail(34,tag);return 0;}u32 state=get((void*)0x6c5c5c)+get((void*)(0x6cb942+y*4))+get((void*)(0x6cb8c2+(z/2)*4))+x;if(!readable(state,1)){HeapFree(producer_heap(),0,out);fail(34,tag);return 0;}u32 hidden=(i32)(signed char)*(u8*)state==(i32)get((void*)0x5e18f0);
  if(!meta[11]||!hidden){i32 u=wrap((i32)x+(i32)(w/2)-(i32)meta[3],w),v=wrap((i32)y+(i32)(h/2)-(i32)meta[4],h),old=u;
   if(meta[9]==1){u=v;v=(i32)meta[5]-old;}else if(meta[9]==2){u=(i32)meta[5]-u;v=(i32)meta[6]-v;}else if(meta[9]==3){u=(i32)meta[6]-v;v=old;}
   i32 row=(u+v)/2;u32 rows=get((void*)(p+0xf3));
   for(i32 d=-2;d<=2;++d){long long entry=(long long)rows+((long long)row+d)*4,expected=(long long)dst->pixels+((long long)(i32)meta[8]*dst->stride+(i32)meta[7]+((long long)row+d)*dst->stride)*2;
    if(entry<=0||entry>0xffffffffll||expected<0||expected>0xffffffffll||!readable((u32)entry,4)||get((void*)(u32)entry)!=(u32)expected){HeapFree(producer_heap(),0,out);fail(36,tag);return 0;}
   }
  }
  put(out+244+i*12,x);put(out+248+i*12,y);put(out+252+i*12,hidden);}
 return out;
}
static u8* request(const struct Frame* f){
 u32 r[24]={0};r[22]=f->caller;u32 tag=f->tag;
 if(tag>=46)return minimap_owned?owned_map_overlay(f):map_points(f);
 if(tag==3){r[2]=MNM_PRODUCER_BIND;struct Surface* s=0;for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].object&&surfaces[i].pixels==f->ecx){s=surfaces+i;break;}if(!s){fail(1,f->caller);return 0;}extent(r,s);r[7]=f->edx;r[14]=f->args[0];}
 else if(tag==4){r[2]=MNM_PRODUCER_CLIP;r[10]=f->edx;r[11]=f->ecx;r[12]=f->args[0];r[13]=f->args[1];}
 else if(tag==5||tag==6){r[2]=MNM_PRODUCER_FILL;struct Surface* s=object(f->ecx);if(!s){fail(2,f->caller);return 0;}extent(r,s);r[14]=f->args[tag==5?0:1]&0xffff;if(tag==5){r[12]=s->width;r[13]=s->height;}else{if(!readable(f->args[0],16)){fail(3,tag);return 0;}for(u32 i=0;i<4;++i)r[10+i]=get((void*)(f->args[0]+i*4));}}
 else if(tag==7||tag==8){r[2]=MNM_PRODUCER_COPY;struct Surface* src=object(f->ecx),*dst=object(f->args[0]);if(!src||!dst){fail(4,f->caller);return 0;}extent(r,dst);r[4]=src->id;r[8]=f->args[1];r[9]=f->args[2];r[12]=src->width;r[13]=src->height;r[14]=tag==8?get((void*)(f->ecx+44))&0xffff:0xffffffff;checkpoint(src,1);}
 else if(tag==41){
  struct Surface* src=object(f->ecx),*dst=object(f->args[0]);if(!src||!dst||!readable(f->args[1],16)||!readable(f->args[2],16)){fail(26,tag);return 0;}r[2]=MNM_PRODUCER_COPY;extent(r,dst);r[4]=src->id;for(u32 i=0;i<4;++i)r[10+i]=get((void*)(f->args[1]+i*4));r[8]=get((void*)f->args[2]);r[9]=get((void*)(f->args[2]+4));if(get((void*)(f->args[2]+8))-r[8]!=r[12]-r[10]||get((void*)(f->args[2]+12))-r[9]!=r[13]-r[11]){fail(27,tag);return 0;}r[14]=get((void*)(f->ecx+44))&0xffff;checkpoint(src,1);
 }
 else if(tag==45){
  if(!readable(f->ecx,0x61cd)){fail(29,tag);return 0;}u32 cells=get((void*)(f->ecx+0x29));if(!cells)return 0;
  u32 base=get((void*)(f->ecx+0xdf)),first=get((void*)(f->ecx+0xe7));struct Surface* dst=0;for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].object&&surfaces[i].pixels==base){dst=surfaces+i;break;}u32 w=get((void*)(f->ecx+0x6199)),h=get((void*)(f->ecx+0x619d)),fog=get((void*)0x5e1404);struct Surface* src=object(f->ecx+0x61a9);
  u32 bad=(!dst?1:0)|(!w||w>256||w%2?2:0)|(!h||h>256?4:0)|(!readable(cells,w*h*16)?8:0)|(get((void*)(f->ecx+0xc5))>(minimap_owned?3u:0u)?16:0)|(!src&&fog?32:0)|(dst&&get((void*)(f->ecx+0xeb))!=dst->stride?64:0)|(first<base||(first-base)%2?128:0)|(dst&&!dst->stride?256:0);if(bad){fail(30,bad);return 0;}
  r[16]=get((void*)(f->ecx+0xc5));r[2]=MNM_PRODUCER_MINIMAP;r[4]=src?src->id:0;r[14]=w;r[15]=h;r[17]=fog!=0;r[10]=get((void*)(f->ecx+0x61a1));r[11]=get((void*)(f->ecx+0x61a5));extent(r,dst);u32 offset=(first-base)/2;r[8]=offset%dst->stride;r[9]=offset/dst->stride;r[18]=w*h*3;r[0]=96+r[18];u8* out=HeapAlloc(producer_heap(),0,r[0]);if(!out){fail(7,tag);return 0;}copy(out,r,96);for(u32 i=0;i<w*h;++i){out[96+i*3]=*(u8*)(cells+i*16+12);out[97+i*3]=*(u8*)(cells+i*16+13);u32 state=get((void*)(cells+i*16+8));if(fog&&!readable(state,1)){HeapFree(producer_heap(),0,out);fail(31,tag);return 0;}out[98+i*3]=fog&&(i32)(signed char)*(u8*)state==(i32)get((void*)0x5e18f0);}return out;
 }
 else if(tag==44){struct Surface* dst=object(f->ecx);if(!dst){fail(28,tag);return 0;}r[2]=MNM_PRODUCER_FADE;extent(r,dst);r[14]=get((void*)0x6e1f88);}
 else if(tag==9||tag==35||tag==40){r[2]=tag==9?MNM_PRODUCER_JPEG:MNM_PRODUCER_BMP;struct Surface* s=object(f->ecx);if(!s){fail(5,f->caller);return 0;}extent(r,s);r[14]=get((void*)0x6e1f88);if(tag==35){r[8]=f->args[1];r[9]=f->args[2];}u32 n=0;while(n<1024&&readable(f->args[0]+n,1)&&*(u8*)(f->args[0]+n))++n;if(n==1024||!readable(f->args[0]+n,1)){fail(6,tag);return 0;}u8* out=HeapAlloc(producer_heap(),0,96+n+1);if(!out){fail(7,tag);return 0;}r[0]=96+n+1;r[18]=n+1;copy(out,r,96);copy(out+96,(void*)f->args[0],n+1);return out;}
 else if(tag==32||tag==33||(tag>=37&&tag<=39)||tag==42||tag==43){
  struct Surface* src=object(f->ecx),*dst=object(f->args[0]);if(!src||!dst||!readable(f->args[1],16)){fail(22,tag);return 0;}r[2]=MNM_PRODUCER_COPY;extent(r,dst);r[4]=src->id;r[8]=f->args[2];r[9]=f->args[3];for(u32 i=0;i<4;++i)r[10+i]=get((void*)(f->args[1]+i*4));r[14]=tag==38||tag==39||tag==42?get((void*)(f->ecx+44))&0xffff:0xffffffff;checkpoint(src,1);
 }
 else if(tag==31){
  struct Surface* dst=object(f->ecx);u32 obj=f->args[0];if(!dst||!readable(obj,8)){fail(23,tag);return 0;}u32 info=get((void*)obj),pixels=get((void*)(obj+4));if(!readable(info,40)){fail(23,tag);return 0;}u32 w=get((void*)(info+4)),h=get((void*)(info+8)),bits=*(u16*)(info+14);if(get((void*)info)!=40||*(u16*)(info+12)!=1||get((void*)(info+16))||!w||w>2048||!h||h>2048||(bits!=1&&bits!=4&&bits!=8&&bits!=24)){fail(24,tag);return 0;}u32 palette=bits==24?0:(1u<<bits)*4,n=((w*bits+31)/32)*4*h;if(n>2097152||!readable(info,40+palette)||!readable(pixels,n)){fail(25,tag);return 0;}r[2]=MNM_PRODUCER_DIB;extent(r,dst);r[14]=bits==24?1:0;r[19]=40+palette;r[20]=n;r[18]=r[19]+n;r[0]=96+r[18];u8* out=HeapAlloc(producer_heap(),0,r[0]);if(!out){fail(7,tag);return 0;}copy(out,r,96);copy(out+96,(void*)info,r[19]);copy(out+96+r[19],(void*)pixels,n);return out;
 }
 else if(tag==30){
  struct Surface* s=object(f->args[0]);if(!s||!readable(f->ecx,20)){fail(20,tag);return 0;}u32 source=get((void*)f->ecx),n=get((void*)(f->ecx+4)),palette=get((void*)(f->ecx+8));if(n<128||n>1048576||!readable(source,n)||!readable(palette,768)){fail(21,tag);return 0;}r[2]=MNM_PRODUCER_PCX;extent(r,s);r[14]=get((void*)(f->ecx+12));r[15]=get((void*)(f->ecx+16));r[17]=get((void*)0x6e1f88);r[19]=n;r[20]=768;r[18]=n+768;r[0]=96+r[18];u8* out=HeapAlloc(producer_heap(),0,r[0]);if(!out){fail(7,tag);return 0;}copy(out,r,96);copy(out+96,(void*)source,n);copy(out+96+n,(void*)palette,768);return out;
 }
 else if(tag>=27&&tag<=29){
  struct Surface* s=object(f->ecx);if(!s||!readable(f->args[0],16)){fail(19,tag);return 0;}r[2]=tag==27?MNM_PRODUCER_BEVEL:tag==28?MNM_PRODUCER_HALO:MNM_PRODUCER_WASH;extent(r,s);for(u32 i=0;i<4;++i)r[10+i]=get((void*)(f->args[0]+i*4));if(tag==27){u32 ox=get((void*)(f->ecx+32)),oy=get((void*)(f->ecx+36));r[10]-=ox;r[12]-=ox;r[11]-=oy;r[13]-=oy;r[14]=f->args[1];r[15]=f->args[2];r[16]=f->args[3];}r[17]=get((void*)0x6e1f88);
 }
 else if(tag==26){
  struct Surface* s=bound();if(!s||!readable(f->ecx,20)){fail(18,tag);return 0;}r[2]=MNM_PRODUCER_RGB_ADD;extent(r,s);clip(r);r[8]=f->args[0];r[9]=f->args[1];r[14]=get((void*)0x6e1f88);u8* out=HeapAlloc(producer_heap(),0,116);if(!out){fail(7,tag);return 0;}r[0]=116;r[18]=20;copy(out,r,96);copy(out+96,(void*)f->ecx,20);return out;
 }
 else if(tag==10||hooks[tag].raster>=0){
  r[2]=tag==10?MNM_PRODUCER_FONT:MNM_PRODUCER_RASTER;struct Surface* s=bound();if(!s){fail(8,f->caller);return 0;}extent(r,s);clip(r);
  i32 producer=hooks[tag].raster;u32 frame,x,y,shade=0,ordinal=0;
  if(tag==10||tag==11||(producer>=4&&producer<=7)||producer==10){frame=f->ecx;x=f->edx;y=f->args[0];shade=f->args[1];ordinal=f->args[2];}
  else{frame=f->args[0];x=f->args[1];y=f->args[2];if(producer>0&&producer<4){shade=f->args[3];ordinal=f->args[4];}}
  if(!readable(frame,40)){fail(9,tag);return 0;}u32 size=get((void*)frame),indexed=get((void*)(frame+28))!=0xffffffff;
  if(size<40||size>1048576||!readable(frame,size)){fail(14,tag);return 0;}
  r[8]=x;r[9]=y;r[14]=producer==4?1:producer==5?4:producer==6?2:producer==7||producer==9?3:producer==10?5:0;r[15]=(u32)producer;r[16]=shade;r[17]=indexed;r[19]=size;r[20]=tag==10?272:producer==7||producer==9?64:indexed?512:0;
  u32 total=96+size+r[20];u8* out=HeapAlloc(producer_heap(),0,total);if(!out){fail(7,tag);return 0;}r[0]=total;r[18]=size+r[20];copy(out,r,96);copy(out+96,(void*)frame,size);put(out+96+28,0);
  u8* colours=out+96+size;
  if(tag==10){put(out+15*4,shade);put(out+16*4,ordinal);put(out+17*4,f->args[3]);}
  else if(r[20]==512){
   if(producer==0||producer==10)zero(colours,512);
   else{u32 node=get((void*)(frame+28));if(ordinal>18)goto palette_fail;for(u32 i=0;i<ordinal;++i){if(!readable(node,12))goto palette_fail;node=get((void*)node);}if(!readable(node,12))goto palette_fail;u32 shift=get((void*)(node+4)),neutral=get((void*)(node+8));if(shift>7||neutral>127)goto palette_fail;i32 q=(i32)shade>>shift;if(q<0&&shift)++q;i32 table=(i32)neutral+q;u32 step=producer==2?4:2;if(table<0||table>254||!readable(node+12+(u32)table*256*step,256*step))goto palette_fail;for(u32 i=0;i<256;++i){colours[i*2]=*(u8*)(node+12+(u32)table*256*step+i*step);colours[i*2+1]=*(u8*)(node+13+(u32)table*256*step+i*step);}}
  }
  return out;
 palette_fail:HeapFree(producer_heap(),0,out);fail(15,tag);return 0;
 }
 else return 0;
 u8* out=HeapAlloc(producer_heap(),0,96);if(!out){fail(7,tag);return 0;}r[0]=96;copy(out,r,96);return out;
}
static void raster_state(u8* record,u32 tag){u32* r=(u32*)record;if(r[2]!=MNM_PRODUCER_RASTER||r[14]!=3)return;u8* data=record+96+r[19];u32 phase=get((void*)0x6c4830);i32 left=(i32)r[8]-(i32)get(record+96+12),top=(i32)r[9]-(i32)get(record+96+16);u32 w=get(record+96+4);i32 producer=(i32)r[15];u32 period=16;
     if(producer==9){i32 skip=top<(i32)r[11]?(i32)r[11]-top:0;for(u32 j=0;j<16;++j)put(data+j*4,get((void*)(0x656640+((j+phase+1-(u32)skip)&15)*4)));}
     else if(left<(i32)r[10]||(long long)left+w>(i32)r[12]){for(u32 j=0;j<16;++j){i32 d=8-(i32)((j+phase)&15);put(data+j*4,d<0?-d:d);}}
     else{u32 amplitude=r[16]?r[16]:get((void*)0x5e15f8);if(!amplitude||amplitude>8||get((void*)0x5f14d0)==0x7d00){fail(17,tag);amplitude=1;}period=amplitude*2;for(u32 j=0;j<16;++j)put(data+j*4,get((void*)(0x5f14d0+(amplitude*16+(j+phase)%period)*4)));}r[16]=period;
    
}
static void frame_enter(u32* registers,u32 tag){
 u32* args=registers+9;struct Frame f={0};f.tag=tag;f.caller=args[0];f.ecx=registers[6];f.edx=registers[5];f.sp=(u32)args+4+(tag==100?0:hooks[tag].pop);for(u32 i=0;i<6;++i)f.args[i]=args[i+1];
 if(tag!=100){f.primitive=tag==10||hooks[tag].raster>=0;if(!f.primitive||!depth)f.record=request(&f);if(f.primitive)++depth;}
 for(u32 i=0;i<MAX_FRAMES;++i)if(!frames[i].sp){frames[i]=f;args[0]=(u32)producer_leave;return;}
 if(f.record)HeapFree(producer_heap(),0,f.record);fail(16,tag);canvas_producers_close();
}
#include "world_producer_bypass.c"
int producer_enter_observe(u32* registers,u32 tag){
 u32 error=GetLastError();
 if(enabled&&!stopped){
  if(batch_guard_open&&hooks[tag].raster<0&&tag!=4){fail(45,tag);ExitProcess(95);}
  if(!depth&&((bypass_limit&&queue==bypass_queue&&bypass_completed<bypass_limit&&(tag==13||tag==14))||(raster_active()&&hooks[tag].raster>=0))){
   if(raster_active()&&(!raster_caller((registers+9)[0],tag)||bypass_completed>=MNM_WORLD_RASTER_MAX)){fail(43,tag);ExitProcess(95);}
   if(raster_active()&&tag==21&&get((void*)0x5f14d0)==0x7d00){
    /* Preserve original non-drawing lazy table preparation, on a zero-size frame. */
    u32 empty[11]={44,0,0};typedef u32 (__attribute__((fastcall)) *Wave)(void*,u32,u32,u32);((Wave)producer_trampolines[21])(empty,0,0,1);++raster_preparations;
   }
   struct Frame f={0};u32* args=registers+9;f.tag=tag;f.caller=args[0];f.ecx=registers[6];f.edx=registers[5];for(u32 i=0;i<6;++i)f.args[i]=args[i+1];
   u8* data=request(&f);if(!data){fail(40,tag);ExitProcess(95);}
   u32* r=(u32*)data;struct Surface* s=bound();
   if(r[2]!=MNM_PRODUCER_RASTER||(!raster_active()&&(r[14]||r[15]!=1||!r[17]))||!s||r[3]!=s->id){HeapFree(producer_heap(),0,data);fail(41,tag);ExitProcess(95);}
   r[21]=registers[7];
   if(raster_active()){raster_state(data,tag);r[21]=0;if(tag==12||tag==23||tag==24){long long left=(i32)r[8]-(i32)get(data+96+12);u32 w=get(data+100),h=get(data+104);r[21]=w&&h&&(left<(i32)r[10]||left+w>=(i32)r[12]);}}
   r[1]=sequence+1;
   if(!batch_enabled&&!bypass_notify(r,hooks[tag].address)){HeapFree(producer_heap(),0,data);fail(42,tag);ExitProcess(95);}
   emit(r,data+96,r[18]);
   if(stopped||!(batch_enabled?batch_accept(r,s,hooks[tag].address):bypass_reply(r,s,hooks[tag].address))){HeapFree(producer_heap(),0,data);fail(42,tag);ExitProcess(95);}
   ++s->dirty;if(raster_active())registers[7]=r[21];HeapFree(producer_heap(),0,data);SetLastError(error);return 1;
  }
  frame_enter(registers,tag);
 }
 SetLastError(error);return 0;
}
void canvas_producers_queue(u32* registers){u32 error=GetLastError();if(enabled&&!stopped){raster_world_open=1;++queue;struct Surface* s=bound();checkpoint(s,2);u32 r[24]={0};r[2]=MNM_PRODUCER_QUEUE_ENTRY;extent(r,s);r[14]=queue;emit(r,0,0);frame_enter(registers,100);}SetLastError(error);}
/* Activate only after all entry observers have finished their snapshots and
 * freed temporary process-heap buffers, immediately before original traversal. */
void canvas_producers_protect(void){u32 error=GetLastError();if(enabled&&!stopped&&batch_enabled&&raster_active()&&!batch_start(bound())){fail(46,batch_reject);ExitProcess(95);}SetLastError(error);}
void producer_leave_observe(u32* registers){
 u32 error=GetLastError(),sp=(u32)registers+40;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp==sp){struct Frame f=frames[i];frames[i].sp=0;registers[9]=f.caller;
  if(f.primitive&&depth)--depth;
  if(!stopped){
   if(f.tag==0){struct Surface* s=object(f.ecx);if(s){u32 r[24]={0};r[2]=MNM_PRODUCER_RELEASE;extent(r,s);emit(r,0,0);zero(s,sizeof(*s));}for(u32 j=0;j<MAX_SURFACES;++j)if(!surfaces[j].object){s=surfaces+j;s->object=f.ecx;s->id=++next_id;s->width=get((void*)(f.ecx+16));s->height=get((void*)(f.ecx+20));s->stride=get((void*)(f.ecx+24))/2;u32 r[24]={0};r[2]=MNM_PRODUCER_CREATE;extent(r,s);r[14]=f.args[0];r[21]=registers[7];r[22]=f.caller;emit(r,0,0);break;}}
   else if(f.tag==1){struct Surface* s=object(f.ecx);if(s){u32 r[24]={0};r[2]=MNM_PRODUCER_RELEASE;extent(r,s);emit(r,0,0);zero(s,sizeof(*s));}}
   else if(f.tag==2){struct Surface* s=object(f.ecx);if(s){s->pixels=registers[7];s->stride=get((void*)(f.ecx+24))/2;checkpoint(s,3);}}
   else if(f.tag==100){struct Surface* s=bound();if(batch_enabled&&raster_active()&&!batch_finish(s)){fail(47,queue);ExitProcess(95);}checkpoint(s,4);u32 r[24]={0};r[2]=MNM_PRODUCER_QUEUE_RETURN;extent(r,s);r[14]=queue;if(raster_active()){r[15]=bypass_completed;r[16]=raster_preparations;if(batch_enabled){r[17]=batch_transfers;r[19]=batch_count;r[20]=1;}}emit(r,0,0);raster_world_open=0;if(queue>=limit){canvas_producers_close();char name[260];u32 n=0;while(directory[n]){name[n]=directory[n];++n;}const char* suffix="\\canvas-producers.done";while(*suffix)name[n++]=*suffix++;name[n]=0;HANDLE done=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(done!=(HANDLE)-1){u32 mark[8]={0};copy(mark,"MNMPDONE",8);mark[2]=1;mark[3]=queue;mark[4]=sequence;mark[5]=failures;mark[6]=oracle;mark[7]=bytes;write(done,mark,32);CloseHandle(done);}}}
   if(f.record){u32* r=(u32*)f.record;r[21]=registers[7];
    if(minimap_owned&&f.tag==45)r[19]=get((void*)(f.ecx+0xff));
    if(minimap_owned&&f.tag==47){struct Surface* d=map_destination(f.ecx);if(!d||registers[7]<d->pixels||(registers[7]-d->pixels)%2){fail(35,f.tag);}else r[21]=(registers[7]-d->pixels)/2;}
    if(minimap_owned&&f.tag==48){r[19]=get((void*)(f.ecx+0x6199));r[20]=get((void*)(f.ecx+0x619d));}
    if(r[2]==MNM_PRODUCER_FONT){u8* data=f.record+96+r[19];put(data,get((void*)0x6e1f88));for(u32 j=0;j<3;++j)put(data+4+j*4,*(u16*)(0x6e1f7c+j*2));copy(data+16,(void*)0x6f5dac,256);}
    else if(r[2]==MNM_PRODUCER_RASTER)raster_state(f.record,f.tag);
    emit(r,f.record+96,r[18]);for(u32 j=0;j<MAX_SURFACES;++j)if(surfaces[j].id==r[3]){struct Surface* s=surfaces+j;if((r[2]>=MNM_PRODUCER_FILL&&r[2]<=MNM_PRODUCER_RASTER)||r[2]>=MNM_PRODUCER_RGB_ADD){++s->dirty;if(r[2]==MNM_PRODUCER_FILL||r[2]==MNM_PRODUCER_JPEG||r[2]==MNM_PRODUCER_COPY||r[2]==MNM_PRODUCER_FADE||r[2]==MNM_PRODUCER_MINIMAP)checkpoint(s,5);}break;}
   }
  }
  if(f.tag==45)for(u32 k=0;k<MAX_FRAMES;++k)if(frames[k].sp&&(frames[k].tag==46||frames[k].tag>=49)&&frames[k].record){u32* q=(u32*)frames[k].record;if(minimap_owned){q[19]=get((void*)(frames[k].ecx+0xf7));q[20]=get((void*)(frames[k].ecx+0xfb));}emit(q,frames[k].record+96,q[18]);struct Surface* dst=map_destination(frames[k].ecx);if(dst)++dst->dirty;HeapFree(producer_heap(),0,frames[k].record);frames[k].record=0;break;}
  if(f.record)HeapFree(producer_heap(),0,f.record);if(f.tag==100&&minimap_pace&&queue<limit)Sleep(250);SetLastError(error);return;
 }
 ExitProcess(94);
}
int canvas_producers_install(void){
 char flag[2],number[8];if(!GetEnvironmentVariableA("MNM_CANVAS_PRODUCERS",flag,2))return 1;
 if(GetEnvironmentVariableA("MNM_CANVAS_STARTUP",flag,2)||GetEnvironmentVariableA("MNM_SCENE_WORLD",flag,2))return 0;
#ifndef MNM_SCENE_SELFTEST
 if((u32)GetModuleHandleA(0)!=0x400000)return 0;
#endif
 SetLastError(0);u32 owned_n=GetEnvironmentVariableA("MNM_MINIMAP_OWNED",flag,2);if((owned_n&&(owned_n!=1||flag[0]!='1'))||(!owned_n&&GetLastError()!=203))return 0;minimap_owned=owned_n!=0;
 SetLastError(0);u32 pace_n=GetEnvironmentVariableA("MNM_MINIMAP_INPUT_FIXTURE",flag,2);if((pace_n&&(pace_n!=1||flag[0]!='1'||!minimap_owned))||(!pace_n&&GetLastError()!=203))return 0;minimap_pace=pace_n!=0;
 oracle_byte_limit=1024u*1024u*1024u;
 SetLastError(0);u32 budget_n=GetEnvironmentVariableA("MNM_CANVAS_ORACLE_MIB",number,sizeof(number));
 if(budget_n){if(budget_n>=sizeof(number)||!oracle_configure(number,budget_n))return 0;}
 else if(GetLastError()!=203)return 0; /* ERROR_ENVVAR_NOT_FOUND; empty refuses. */
 limit=1;u32 n=GetEnvironmentVariableA("MNM_SCENE_SAMPLES",number,8);if(n){if(n>=sizeof(number))return 0;limit=0;for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return 0;limit=limit*10+number[i]-'0';}if(!limit||(limit>16&&limit!=MNM_PRODUCER_V3_QUEUES))return 0;}
 if(!bypass_init(limit)||!raster_init(limit)||(bypass_limit&&raster_first))return 0;
 if(!batch_init())return 0;
 if(limit>16&&(!batch_enabled||minimap_owned||raster_first!=1||raster_last!=limit))return 0;
 n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory))return 0;
 for(u32 i=0;i<HOOKS;++i){struct Hook* h=hooks+i;if(!readable(h->address,h->length))return 0;for(u32 j=0;j<h->length;++j)if(*(u8*)(h->address+j)!=h->bytes[j])return 0;u8* t=VirtualAlloc(0,h->length+5,0x3000,0x40);if(!t)return 0;copy(t,(void*)h->address,h->length);if(i==9)put(t+5,0x58b660-(u32)t-9);t[h->length]=0xe9;put(t+h->length+1,h->address+h->length-(u32)t-h->length-5);producer_trampolines[i]=(u32)t;}
 char name[260];u32 i=0;while(directory[i]){name[i]=directory[i];++i;}const char* suffix="\\canvas-producers.bin";while(*suffix)name[i++]=*suffix++;name[i]=0;file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(file==(HANDLE)-1){file=0;return 0;}
 u32 header[16]={0};copy(header,limit>16?MNM_PRODUCER_V3_MAGIC:minimap_owned?"MNMPRO02":MNM_PRODUCER_MAGIC,8);header[2]=limit>16?3:minimap_owned?2:1;header[3]=64;header[4]=MNM_PRODUCER_BUILD;header[5]=limit>16?MNM_PRODUCER_V3_MAX_RECORDS:minimap_owned?MNM_PRODUCER_V2_MAX_RECORDS:MNM_PRODUCER_MAX_RECORDS;header[6]=limit;if(!write(file,header,64))return 0;bytes=64;
 for(u32 j=0;j<HOOKS;++j){struct Hook* h=hooks+j;u32 old,unused;if(!VirtualProtect((void*)h->address,h->length,0x40,&old))return 0;u8 patch[10]={0xe9};put(patch+1,(u32)h->entry-h->address-5);for(u32 k=5;k<h->length;++k)patch[k]=0x90;copy((void*)h->address,patch,h->length);FlushInstructionCache(GetCurrentProcess(),(void*)h->address,h->length);VirtualProtect((void*)h->address,h->length,old,&unused);}
 enabled=1;return 1;
}
