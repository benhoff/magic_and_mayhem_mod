/* Actual facade output, independent five-colour oracle across all 25 canonical
 * format pairs, nearest-neighbour enlargement/reduction and raw source keys. */
#define start resource_baseline_start
#include "native-draw-resource-probe.c"
#undef start
static const u32 colours[5][5]={
 {0,1,2,3,4},{0,0x7c00,0x03e0,0x001f,0x7fff},
 {0,0xf800,0x07e0,0x001f,0xffff},
 {0,0xff0000,0xff00,0xff,0xffffff},{0,0xff0000,0xff00,0xff,0xffffff}};
static void* make_surface(void* draw,u32 format){
 u32 d[31];desc(d,format);void* s=0;
 require(!((Surface)table(draw)[6])(draw,d,&s,0),60);
 if(!format){static const u32 entries[256]={0,0xff,0xff00,0xff0000,0xffffff};void* p=0;
  require(!((i32(WIN*)(void*,u32,const u32*,void**,void*))table(draw)[5])(draw,0x44,entries,&p,0),61);
  require(!((i32(WIN*)(void*,void*))table(s)[31])(s,p),62);release(p);}
 return s;
}
static void paint(void* s,u32 format,int pattern){
 u32 d[31]={108};require(!((Lock)table(s)[25])(s,0,d,1,0),63);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u32 v=colours[format][pattern?(x+y*13)%5:3];u8* p=(u8*)d[9]+y*d[4]+x*(d[21]/8);
  for(u32 b=0;b<d[21]/8;++b)p[b]=(u8)(v>>(8*b));}
 require(!((i32(WIN*)(void*,void*))table(s)[32])(s,(void*)d[9]),64);
}
void start(void){
 char path[1024];require(GetEnvironmentVariableA("MNM_TEST_LIBRARY",path,sizeof(path))>0,65);
 void* module=LoadLibraryA(path);require(module!=0,66);Create create=GetProcAddress(module,"NativeDrawCreate");
 void* draw=0;require(create&&!create(0,&draw,0),67);
 for(u32 sf=0;sf<5;++sf)for(u32 df=0;df<5;++df){
  void* source=make_surface(draw,sf);void* target=make_surface(draw,df);paint(source,sf,1);
  for(u32 mode=0;mode<3;++mode)for(u32 keyed=0;keyed<2;++keyed){
   paint(target,df,0);u32 key[2]={colours[sf][1],colours[sf][1]};
   require(!((i32(WIN*)(void*,u32,const u32*))table(source)[29])(source,8,key),68);
   i32 sr[4]={0,0,mode==2?7:13,mode==2?5:9};i32 dr[4]={0,0,mode==1?7:13,mode==1?5:9};
   if(!mode)require(!((i32(WIN*)(void*,u32,u32,void*,const i32*,u32))table(target)[7])(target,0,0,source,sr,0x10|keyed),69);
   else require(!((i32(WIN*)(void*,const i32*,void*,const i32*,u32,const u32*))table(target)[5])(target,dr,source,sr,keyed?0x8000:0,0),70);
   u32 d[31]={108};require(!((Lock)table(target)[25])(target,0,d,0x10,0),71);
   for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u32 colour=3;
    if(x<(u32)dr[2]&&y<(u32)dr[3]){u32 sx=x*(u32)sr[2]/(u32)dr[2],sy=y*(u32)sr[3]/(u32)dr[3];u32 selected=(sx+sy*13)%5;
     if(!keyed||selected!=1)colour=selected;}
    const u8* p=(const u8*)d[9]+y*d[4]+x*(d[21]/8);u32 got=0;
    for(u32 b=0;b<d[21]/8;++b)got|=(u32)p[b]<<(8*b);
    require(got==colours[df][colour],72);}
   require(!((i32(WIN*)(void*,void*))table(target)[32])(target,(void*)d[9]),73);
  }
  require(!release(source)&&!release(target),74);
 }
 require(!release(draw),75);ExitProcess(0);
}
