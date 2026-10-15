/* Native ownership policies, derived from supplied source bytes. No game or
 * presented-driver output supplies expected resource contents. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN GetDesktopWindow(void);
API void* WIN CreateDIBSection(void*,const void*,u32,void**,HANDLE,u32);
API void* WIN SelectObject(void*,void*);
API int WIN DeleteObject(void*);
API u32 WIN GetDIBColorTable(void*,u32,u32,u32*);
typedef u32 (WIN *Ref)(void*);
typedef u32 (WIN *Count)(void);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Surface)(void*,const u32*,void**,void*);
typedef i32 (WIN *Lock)(void*,const i32*,u32*,u32,HANDLE);
typedef i32 (WIN *Unary)(void*);
typedef i32 (WIN *Plane)(const void*,u32*,u32);
typedef i32 (WIN *Copy)(void*,i32,i32,i32,i32,void*,i32,i32,u32);
static u32 checks,formats,palette_words,visible_bytes,padding_bytes;
static void** table(void* p){return *(void***)p;}
static void require(int ok,u32 stage){++checks;if(!ok)ExitProcess(stage);}
static u32 release(void* p){return ((Ref)table(p)[2])(p);}
static void desc(u32* d,u32 format){
 for(u32 i=0;i<31;++i)d[i]=0;
 d[0]=108;d[1]=0x1007;d[2]=9;d[3]=13;d[18]=32;d[19]=format?0x40:0x60;
 d[21]=format==0?8:format<3?16:format==3?24:32;
 d[22]=format==0?0:format==1?0x7c00:format==2?0xf800:0xff0000;
 d[23]=format==0?0:format==1?0x3e0:format==2?0x7e0:0xff00;
 d[24]=format==0?0:format<3?31:255;d[26]=0x840;
}
static const u8 surface4[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};
static void palette(void* draw,void** out,u32 seed){
 u32 entries[256];for(u32 i=0;i<256;++i)entries[i]=((i+seed)&255)|((255-i)<<8)|((i*3%256)<<16)|((i%8)<<24);
 require(!((i32 (WIN *)(void*,u32,const u32*,void**,void*))table(draw)[5])(draw,0x44,entries,out,0),210);
}
static void run_format(Create create,Plane plane,Copy copy,u32 format){
 void *draw=0,*s=0,*alias=0,*dc=0,*p=0,*q=0;u32 d[31],fields[6],colors[256],before[256];desc(d,format);
 require(!create(0,&draw,0),211);
 require(!((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,0,8),212);
 require(!((Surface)table(draw)[6])(draw,d,&s,0),213);
 require(!((i32 (WIN *)(void*,const u8*,void**))table(s)[0])(s,surface4,&alias),214);
 if(!format){
  palette(draw,&p,0);palette(draw,&q,1);
  require(!((i32 (WIN *)(void*,void*))table(alias)[31])(alias,p),215);
  void* acquired=0;require(!((i32 (WIN *)(void*,void**))table(s)[20])(s,&acquired)&&acquired==p,216);
  require(release(p)==2 && release(acquired)==1,217);
 }
 require(!((Lock)table(s)[25])(s,0,d,1,0),218);
 u8* pixels=(u8*)d[9];u32 pitch=d[4],row_bytes=13*d[21]/8;
 for(u32 i=0;i<pitch*9;++i)pixels[i]=(u8)(i*37+format*17);
 require(plane(pixels,fields,24)==(format==1 || format==2?0:(i32)0x80004001u),219);
 if(format==1 || format==2)require(fields[5]==(format==1),220);
 require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,pixels),221);
 require(!((i32 (WIN *)(void*,void**))table(alias)[17])(alias,&dc),222);
 if(!format){
  require(GetDIBColorTable(dc,0,256,before)==256,223);
  for(u32 i=0;i<256;++i){require(before[i]==((i<<16)|((255-i)<<8)|(i*3%256)),224);++palette_words;}
  u32 replacement=0x07a1b2c3;
  require(!((i32 (WIN *)(void*,u32,u32,u32,const u32*))table(p)[6])(p,0,137,1,&replacement),225);
  require(!((i32 (WIN *)(void*,void*))table(s)[31])(s,q),226);
  require(!((i32 (WIN *)(void*,u32,u32,u32,u32*))table(q)[4])(q,0,0,256,colors),227);
  require(GetDIBColorTable(dc,0,256,colors)==256,228);
  for(u32 i=0;i<256;++i){require(colors[i]==before[i],229);++palette_words;}
  require(release(q)==1,230);
 }
 /* Selecting a different bitmap must never redirect a cached native DIB write. */
 u32 info[13]={40,13,0u-9,0x00100001,3,0,0,0,0,0,0xf800,0x7e0,31};void* bits=0;
 void* foreign=CreateDIBSection(dc,info,0,&bits,0,0);require(foreign && bits,231);
 void* owned=SelectObject(dc,foreign);require(owned && owned!=(void*)-1,232);
 require(copy(dc,0,0,1,1,dc,0,0,0xcc0020)==(i32)0x80000000u,233);
 require(((i32 (WIN *)(void*,void*))table(s)[26])(s,dc)==(i32)0x80070057u,234);
 require(((Unary)table(alias)[27])(alias)==(i32)0x887601aeu,235);
 require(SelectObject(dc,owned)==foreign && DeleteObject(foreign),236);
 require(!((i32 (WIN *)(void*,void*))table(s)[26])(s,dc),237);
 require(!((Unary)table(s)[27])(s) && !((Unary)table(s)[24])(s),238);
 desc(d,format);require(!((Lock)table(s)[25])(s,0,d,1,0),239);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<pitch;++x){require(((u8*)d[9])[y*pitch+x]==(u8)((y*pitch+x)*37+format*17),240);
  if(x<row_bytes)++visible_bytes;else ++padding_bytes;}
 require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)d[9]),241);
 if(!format){
  require(!((i32 (WIN *)(void*,void**))table(s)[17])(s,&dc),242);
  require(GetDIBColorTable(dc,0,256,colors)==256,243);
  for(u32 i=0;i<256;++i){require(colors[i]==((((i+1)&255)<<16)|((255-i)<<8)|(i*3%256)),244);++palette_words;}
  require(!((i32 (WIN *)(void*,void*))table(s)[26])(s,dc),245);
 }
 /* Keyed overlapping copies use the pre-draw source in every native format. */
 u32 key[2]={0,0},bytes_per_pixel=row_bytes/13;
 for(u32 i=0;i<bytes_per_pixel;++i)key[0]|=(u32)(u8)(i*37+format*17)<<(8*i);key[1]=key[0];
 require(!((i32 (WIN *)(void*,u32,const u32*))table(alias)[29])(alias,8,key),194);
 i32 from[4]={0,0,7,5},to[4]={2,2,9,7};
 require(!((i32 (WIN *)(void*,const i32*,void*,const i32*,u32,void*))table(s)[5])(s,to,alias,from,0x1008000,0),195);
 desc(d,format);require(!((Lock)table(s)[25])(s,0,d,1,0),196);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){
  int copied=x>=2 && x<9 && y>=2 && y<7;u32 input=0;
  if(copied)for(u32 byte=0;byte<bytes_per_pixel;++byte)input|=(u32)(u8)(((y-2)*pitch+(x-2)*bytes_per_pixel+byte)*37+format*17)<<(8*byte);
  copied=copied && input!=key[0];
  for(u32 byte=0;byte<bytes_per_pixel;++byte){u32 offset=copied?(y-2)*pitch+(x-2)*bytes_per_pixel+byte:y*pitch+x*bytes_per_pixel+byte;
   require(((u8*)d[9])[y*pitch+x*bytes_per_pixel+byte]==(u8)(offset*37+format*17),197);}
 }
 require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)d[9]),198);
 u32 fx[25]={100};fx[20]=0xdeadbeef;to[0]=0;to[1]=0;to[2]=3;to[3]=2;
 require(!((i32 (WIN *)(void*,const i32*,void*,const i32*,u32,const u32*))table(s)[5])(s,to,0,0,0x1000400,fx),199);
 desc(d,format);require(!((Lock)table(s)[25])(s,0,d,1,0),200);
 for(u32 y=0;y<2;++y)for(u32 x=0;x<3;++x)for(u32 byte=0;byte<bytes_per_pixel;++byte)
  require(((u8*)d[9])[y*pitch+x*bytes_per_pixel+byte]==(u8)(fx[20]>>(8*byte)),201);
 for(u32 y=0;y<9;++y)for(u32 x=row_bytes;x<pitch;++x)require(((u8*)d[9])[y*pitch+x]==(u8)((y*pitch+x)*37+format*17),202);
 require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)d[9]),203);
 require(!release(alias) && !release(s) && !release(draw),246);++formats;
}
static void clip_lifetime(Create create){
 void *draw=0,*s=0,*c=0,*got=0;u32 d[31];desc(d,2);
 require(!create(0,&draw,0) && !((Surface)table(draw)[6])(draw,d,&s,0),247);
 require(!((i32 (WIN *)(void*,u32,void**,void*))table(draw)[4])(draw,0,&c,0),248);
 u32 data[16]={32,1,2,32,0,0,13,9,0,0,3,2,5,3,13,9},out[16],bytes=64;
 require(!((i32 (WIN *)(void*,const u32*,u32))table(c)[7])(c,data,0),249);
 require(!((i32 (WIN *)(void*,void*))table(s)[28])(s,c) && release(c)==1,250);
 require(!((i32 (WIN *)(void*,void**))table(s)[15])(s,&got) && got==c,251);
 u32 too_small=4;out[0]=0xabababab;
 require(((i32 (WIN *)(void*,void*,u32*,u32*))table(c)[3])(c,0,out,&too_small)==(i32)0x88760236u && too_small==64 && out[0]==0xabababab,252);
 require(!((i32 (WIN *)(void*,void*,u32*,u32*))table(c)[3])(c,0,out,&bytes),253);
 for(u32 i=0;i<16;++i)require(out[i]==data[i],254);
 i32 changed=-1;
 require(!((i32 (WIN *)(void*,i32*))table(c)[6])(c,&changed) && changed==1,192);
 require(!((i32 (WIN *)(void*,i32*))table(c)[6])(c,&changed) && changed==0,193);
 data[2]=257;require(((i32 (WIN *)(void*,const u32*,u32))table(c)[7])(c,data,0)==(i32)0x8876006eu,255);
 bytes=64;require(!((i32 (WIN *)(void*,void*,u32*,u32*))table(c)[3])(c,0,out,&bytes) && out[2]==2,156);
 require(!((i32 (WIN *)(void*,u32,void*))table(c)[8])(c,0,GetDesktopWindow()),157);
 bytes=0;require(!((i32 (WIN *)(void*,void*,u32*,u32*))table(c)[3])(c,0,0,&bytes) && bytes>=32,158);
 require(!((i32 (WIN *)(void*,u32,void*))table(c)[8])(c,0,0),159);
 require(release(got)==1 && !release(s) && !release(draw),160);
}
static void flip_lifetime(Create create){
 void *draw=0,*front=0,*back=0,*alias=0,*dc=0;u32 d[31],caps[4]={4};desc(d,2);
 require(!create(0,&draw,0),165);
 require(!((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),0x11),166);
 require(!((i32 (WIN *)(void*,u32,u32,u32))table(draw)[21])(draw,13,9,16),167);
 d[1]=0x21;d[26]=0x218;d[5]=1;
 require(!((Surface)table(draw)[6])(draw,d,&front,0),168);
 require(!((i32 (WIN *)(void*,const u8*,void**))table(front)[0])(front,surface4,&alias),169);
 require(!((i32 (WIN *)(void*,const u32*,void**))table(alias)[12])(alias,caps,&back),170);
 u32 keys[2]={0x1234,0x1234};require(!((i32 (WIN *)(void*,u32,u32*))table(front)[29])(front,8,keys),171);
 keys[0]=keys[1]=0x5678;require(!((i32 (WIN *)(void*,u32,u32*))table(back)[29])(back,8,keys),172);
 for(u32 side=0;side<2;++side){void* s=side?back:front;desc(d,2);if(side)d[0]=124;
  require(!((Lock)table(s)[25])(s,0,d,1,0),173);
  for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x)((u16*)(d[9]+y*d[4]))[x]=(u16)(side*4096+y*13+x);
  require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)d[9]),175);
 }
 require(!((i32 (WIN *)(void*,void**))table(alias)[17])(alias,&dc),176);
 require(!((i32 (WIN *)(void*,void*,u32))table(front)[11])(front,0,1),177);
 require(((i32 (WIN *)(void*,void*,u32))table(front)[11])(front,0,1)==(i32)0x887601aeu,174);
 require(!((i32 (WIN *)(void*,void*))table(front)[26])(front,dc),178);
 require(!((i32 (WIN *)(void*,void*,u32))table(alias)[11])(alias,back,1),179);
 for(u32 side=0;side<2;++side){void* s=side?back:front;desc(d,2);if(side)d[0]=124;
  require(!((Lock)table(s)[25])(s,0,d,1,0),180);
  for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x)require(((u16*)(d[9]+y*d[4]))[x]==(1-side)*4096+y*13+x,181);
  require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,(void*)d[9]),182);
  require(!((i32 (WIN *)(void*,u32,u32*))table(s)[16])(s,8,keys) && keys[0]==(side?0x5678:0x1234),183);
 }
 require(!release(alias) && !release(front),184);
 desc(d,2);d[0]=124;require(!((Lock)table(back)[25])(back,0,d,1,0),185);
 require(*(u16*)d[9]==0 && !((i32 (WIN *)(void*,void*))table(back)[32])(back,(void*)d[9]),186);
 require(!release(back) && !release(draw),187);
}
static void palette_capacity(Create create){
 void* draw=0;void* held[256];require(!create(0,&draw,0),188);
 for(u32 i=0;i<256;++i)palette(draw,&held[i],i);
 void* denied=(void*)0x1234;u32 entries[256];for(u32 i=0;i<256;++i)((volatile u32*)entries)[i]=0;
 require(((i32 (WIN *)(void*,u32,const u32*,void**,void*))table(draw)[5])(draw,0x44,entries,&denied,0)==(i32)0x8007000eu && !denied,189);
 for(u32 i=0;i<256;++i)require(!release(held[i]),190);
 require(!release(draw),191);
}
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");require(module!=0,161);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");Plane plane=(Plane)GetProcAddress(module,"NativeDrawPlaneV1");Copy copy=(Copy)GetProcAddress(module,"NativeDrawBitBltV1");
 require(create && plane && copy,162);
 for(u32 f=0;f<5;++f)for(u32 repeat=0;repeat<16;++repeat)run_format(create,plane,copy,f);
 for(u32 repeat=0;repeat<32;++repeat)clip_lifetime(create);
 for(u32 repeat=0;repeat<32;++repeat)flip_lifetime(create);
 palette_capacity(create);
 const char* names[5]={"NativeDrawObjects","NativeDrawSurfaces","NativeDrawDCs","NativeDrawClippers","NativeDrawPalettes"};
 for(u32 i=0;i<5;++i){Count count=(Count)GetProcAddress(module,names[i]);require(count && !count(),163);}
 u32 result[10]={1,checks,formats,palette_words,visible_bytes,padding_bytes,32,0,32,256},done=0;
 HANDLE f=CreateFileA("resource-results.bin",0x40000000,0,0,1,0x80,0);
 require(f!=(HANDLE)-1 && WriteFile(f,result,sizeof(result),&done,0) && done==sizeof(result),164);CloseHandle(f);ExitProcess(0);
}
