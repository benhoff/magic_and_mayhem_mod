/* Passive installed SFT custody. Original reload retention is observed, not
 * repaired. All original pointers remain private lookup keys. */
#define FDC_BYTES (64u*1024u*1024u)
#define FDC_FONTS 128u
#define FDC_ROWS 131072u
API u32 WIN GetTickCount(void);
static void fdc_source_context(u32* h);
static HANDLE fdc_file;
static u32 fdc_rows,fdc_bytes,fdc_loss,fdc_owner,fdc_fonts,fdc_glyphs,fdc_current[4];
static const u32 fdc_objects[4]={0x6b0400,0x656680,0x6df1c0,0x6a5138};
static struct FdcFont {u32 object,raw,bytes,table,base,count,id,role,superseded,retired;u32 offsets[256];} fdc_font[FDC_FONTS];
static struct FdcCall {u32 sp,caller,object,tag,previous,id;u8 path[256];} fdc_calls[32];
u32 font_trampolines[5];
#define F(n) extern void font_enter_##n(void)
F(0);F(1);F(2);F(3);F(4);
#undef F
extern void font_leave(void);
static void fdc_bad(u32 reason){__sync_bool_compare_and_swap(&fdc_loss,0,reason);}
static u32 fdc_role(u32 object){for(u32 i=0;i<4;++i)if(object==fdc_objects[i])return i+1;return 0;}
static u32 fdc_owned_mask(u32 object){u32 mask=0;const u32 offsets[5]={0,8,12,16,20};if(!readable(object,0x238))return 0xffffffffu;for(u32 i=0;i<5;++i)if(get((void*)(object+offsets[i])))mask|=1u<<i;return mask;}
static int fdc_owned(void){u32 thread=GetCurrentThreadId();__sync_bool_compare_and_swap(&fdc_owner,0,thread);if(fdc_owner!=thread){fdc_bad(1);return 0;}return 1;}
static void fdc_emit(u32* h,const void* body,u32 n){
 if(!fdc_file||fdc_loss)return;
 if(fdc_rows>=FDC_ROWS||n>1048832||fdc_bytes>FDC_BYTES-128-n){fdc_bad(2);return;}
 h[0]=128+n;h[1]=++fdc_rows;h[3]=fdc_owner;h[4]=GetTickCount();h[5]=queue;h[6]=sequence;h[22]=n?mnm_cont_crc32c(body,n):0;
 if(!write(fdc_file,h,128)||(n&&!write(fdc_file,body,n))){fdc_bad(3);return;}fdc_bytes+=128+n;
}
void font_enter_observe(u32* r,u32 tag){
 u32 error=GetLastError();if(!fdc_file||fdc_loss){SetLastError(error);return;}
 if(!fdc_owned()){SetLastError(error);return;}u32* args=r+9;
 struct FdcCall* c=0;for(u32 i=0;i<32;++i)if(!fdc_calls[i].sp){c=fdc_calls+i;break;}
 if(!c){fdc_bad(4);SetLastError(error);return;}
 c->object=tag?fdc_objects[tag-1]:r[6];u32 role=fdc_role(c->object);
 if(!role||!readable(c->object,0x238)){fdc_bad(5);SetLastError(error);return;}
 c->tag=tag;c->caller=args[0];c->previous=fdc_current[role-1];c->id=0;zero(c->path,256);
 if(!tag){
  for(u32 i=0;i<256;++i){if(!readable(args[1]+i,1)){fdc_bad(6);break;}c->path[i]=*(u8*)(args[1]+i);if(!c->path[i])break;if(i==255)fdc_bad(6);}
  if(fdc_fonts>=FDC_FONTS)fdc_bad(7);
  if(fdc_loss){SetLastError(error);return;}
  c->id=++fdc_fonts;struct FdcFont* f=fdc_font+c->id-1;f->object=c->object;f->id=c->id;f->role=role;
 }
 u32 h[32]={0};h[2]=tag?4:1;h[7]=tag?c->previous:c->id;h[8]=role;h[9]=c->previous;h[10]=c->caller;h[11]=fdc_owned_mask(c->object);h[21]=tag?0:256;
 fdc_emit(h,tag?0:c->path,tag?0:256);
 c->sp=(u32)args+4+(tag?0:8);args[0]=(u32)font_leave;SetLastError(error);
}
void font_leave_observe(u32* r){
 u32 error=GetLastError(),sp=(u32)r+40;
 for(u32 i=0;i<32;++i)if(fdc_calls[i].sp==sp){
  struct FdcCall c;copy(&c,fdc_calls+i,sizeof(c));zero(fdc_calls+i,sizeof(c));r[9]=c.caller;
  if(!fdc_owned()||fdc_loss){SetLastError(error);return;}
  u32 role=fdc_role(c.object),h[32]={0};h[2]=c.tag?5:2;h[7]=c.tag?c.previous:c.id;h[8]=role;h[9]=c.previous;h[10]=r[7];h[11]=fdc_owned_mask(c.object);
  if(c.tag){
   if(h[11]||!c.previous){fdc_bad(8);SetLastError(error);return;}
   fdc_font[c.previous-1].retired=1;fdc_current[role-1]=0;fdc_emit(h,0,0);SetLastError(error);return;
  }
  if(!r[7]){h[21]=256;fdc_emit(h,c.path,256);SetLastError(error);return;}
  const u32 raw=get((void*)c.object),n=readable(raw,40)?get((void*)(raw+4)):0;
  if(n<40||n>1048576||!readable(raw,n)||get((void*)(raw+8))!=3){fdc_bad(9);SetLastError(error);return;}
  u32 count=get((void*)(raw+12)),rows=get((void*)(raw+16)),profiles=get((void*)(raw+36)),table=get((void*)(c.object+12)),base=get((void*)(c.object+16)),contours=get((void*)(c.object+8));
  if(!count||count>256||!rows||rows>128||!profiles||profiles>256||table<raw||table-raw>n||count*4>n-(table-raw)||base!=table+count*4||contours<raw||contours-raw>n){fdc_bad(10);SetLastError(error);return;}
  u8* body=HeapAlloc(producer_heap(),0,256+n);if(!body){fdc_bad(11);SetLastError(error);return;}copy(body,c.path,256);copy(body+256,(void*)raw,n);
  struct FdcFont* f=fdc_font+c.id-1;f->raw=raw;f->bytes=n;f->table=table;f->base=base;f->count=count;
  for(u32 k=0;k<count;++k){u32 offset=get((void*)(table+k*4)),at=base-raw+offset;f->offsets[k]=offset;
   if(at>n||n-at<40){fdc_bad(12);break;}u32 bytes=get(body+256+at);if(bytes<40||bytes>n-at){fdc_bad(12);break;}put(body+256+at+28,0);
  }
  if(c.previous)fdc_font[c.previous-1].superseded=1;fdc_current[role-1]=c.id;
  h[12]=n;h[13]=count;h[14]=rows;h[15]=contours-raw;h[16]=table-raw;h[17]=base-raw;
  h[18]=get((void*)(c.object+0x220));h[19]=get((void*)(c.object+0x224));h[20]=get((void*)(c.object+0x230));h[21]=256;
  fdc_emit(h,body,256+n);HeapFree(producer_heap(),0,body);SetLastError(error);return;
 }
 fdc_bad(13);SetLastError(error);
}
static void fdc_glyph(const u32* r,const void* payload,u32 n,u32 original_frame){
 if(!fdc_file||r[2]!=8)return;u32 error=GetLastError();if(fdc_loss||!fdc_owned()){SetLastError(error);return;}
 if(!original_frame||!payload||n!=r[19]+272||!r[1]){fdc_bad(14);SetLastError(error);return;}
 struct FdcFont* found=0;u32 ordinal=0;
 for(u32 i=0;i<fdc_fonts;++i){struct FdcFont* f=fdc_font+i;if(!f->raw||f->retired)continue;
  for(u32 k=0;k<f->count;++k)if(f->base+f->offsets[k]==original_frame){if(found){fdc_bad(15);break;}found=f;ordinal=k;}
 }
 if(!found||!fdc_current[found->role-1]){fdc_bad(16);SetLastError(error);return;}
 u32 h[32]={0};h[2]=3;h[7]=found->id;h[8]=found->role;h[9]=ordinal;h[10]=r[1];h[11]=r[3];h[12]=r[19];h[13]=mnm_cont_crc32c(payload,r[19]);h[14]=queue;
 h[15]=r[8];h[16]=r[9];h[17]=r[15];h[18]=r[16];h[19]=r[17];h[20]=r[22];h[21]=fdc_current[found->role-1];h[23]=found->superseded;
 fdc_source_context(h);fdc_emit(h,0,0);++fdc_glyphs;SetLastError(error);
}
static void fdc_close(void){
 if(!fdc_file)return;u32 error=GetLastError(),h[32]={0};h[0]=128;h[1]=++fdc_rows;h[2]=6;h[3]=fdc_owner;h[4]=GetTickCount();h[7]=fdc_loss;h[8]=fdc_fonts;h[9]=fdc_glyphs;h[12]=fdc_bytes;
 for(u32 i=0;i<4;++i)if(fdc_current[i])++h[10];for(u32 i=0;i<32;++i)if(fdc_calls[i].sp)++h[11];for(u32 i=0;i<fdc_fonts;++i)if(fdc_font[i].superseded&&!fdc_font[i].retired)++h[13];
 write(fdc_file,h,128);CloseHandle(fdc_file);fdc_file=0;SetLastError(error);
}
static int fdc_install(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_FONT_LIFETIME",flag,2);if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1'||!continuous_map)return 0;
 const u32 sites[5]={0x4a5ae0,0x4be8c0,0x4be960,0x4bea00,0x4beaa0};const u8 signatures[5][6]={{0x8b,0x44,0x24,0x04,0x53,0x55},{0xa1,0,4,0x6b,0,0},{0xa1,0x80,0x66,0x65,0,0},{0xa1,0xc0,0xf1,0x6d,0,0},{0xa1,0x38,0x51,0x6a,0,0}};
 void(*entries[5])(void)={font_enter_0,font_enter_1,font_enter_2,font_enter_3,font_enter_4};
 for(u32 i=0;i<5;++i){u32 length=i?5:6;if(!readable(sites[i],length))return 0;for(u32 j=0;j<length;++j)if(*(u8*)(sites[i]+j)!=signatures[i][j])return 0;}
 char name[260];path(name,"font-lifetime-",1,".bin");fdc_file=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(fdc_file==(HANDLE)-1){fdc_file=0;return 0;}
 u32 header[16]={0};copy(header,"MNMFLT01",8);header[2]=2;header[3]=64;header[4]=0x40209ca7;header[5]=FDC_BYTES;header[6]=FDC_FONTS;header[7]=FDC_ROWS;header[8]=128;
 if(!write(fdc_file,header,64))return 0;fdc_bytes=64;
 u8* tramp=VirtualAlloc(0,100,0x3000,0x40);if(!tramp)return 0;
 for(u32 i=0;i<5;++i){u32 length=i?5:6;u8* t=tramp+i*20;copy(t,(void*)sites[i],length);t[length]=0xe9;put(t+length+1,sites[i]+length-(u32)t-length-5);font_trampolines[i]=(u32)t;}
 for(u32 i=0;i<5;++i){u32 old,unused,length=i?5:6;if(!VirtualProtect((void*)sites[i],length,0x40,&old))return 0;u8 patch[6]={0xe9,0,0,0,0,0x90};put(patch+1,(u32)entries[i]-sites[i]-5);copy((void*)sites[i],patch,length);if(!FlushInstructionCache(GetCurrentProcess(),(void*)sites[i],length)||!VirtualProtect((void*)sites[i],length,old,&unused))return 0;}
 return 1;
}
