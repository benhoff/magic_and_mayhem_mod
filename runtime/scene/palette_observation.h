/* Opt-in passive builder/selection custody. Pointers are private lookup keys;
 * only monotonically assigned allocation IDs cross the observation wire. */
#define PAL_NODES 4096u
#define PAL_FRAMES 65536u
#define PAL_BYTES (128u*1024u*1024u)
API u32 WIN GetTickCount(void);
API void* WIN GetProcAddress(void*,const char*);
static HANDLE pal_file;
static u32 pal_rows,pal_bytes,pal_loss,pal_owner,pal_nodes,pal_bindings,pal_full_prefix;
static struct PalNode {u32 pointer,id,mode,count,live,revision;} pal_node[PAL_NODES];
static u32 pal_lookup[8192];
static struct PalChoice {u32 frame,node,ordinal,table,crc,revision;} pal_choice[PAL_FRAMES];
/* Shade-only alternation at different terrain cells is not a new palette
 * version. Retain each first frame/node/table value, every changed value and
 * every frame's allocation/ordinal transition (including A->B->A). */
static struct PalValue {u32 frame,node,table,crc,revision;} pal_value[PAL_FRAMES*4];
static struct PalCall {u32 sp,caller,tag,count,slot,source,id,parent,api_args[7];u8* profile;} pal_call[32];
u32 palette_trampolines[14];
#define P(n) extern void palette_enter_##n(void)
P(0);P(1);P(2);P(3);P(4);P(5);P(6);P(7);P(8);P(9);P(10);P(11);P(12);P(13);
#undef P
extern void palette_leave(void);
static void pal_bad(u32 why){__sync_bool_compare_and_swap(&pal_loss,0,why);}
static int pal_owned(void){u32 owner=GetCurrentThreadId();__sync_bool_compare_and_swap(&pal_owner,0,owner);if(__atomic_load_n(&pal_owner,__ATOMIC_ACQUIRE)!=owner){pal_bad(1);return 0;}return 1;}
static void pal_emit(u32* h,const void* data,u32 n){
 if(!pal_file||pal_loss)return;
 if(n>PAL_BYTES-128||pal_bytes>PAL_BYTES-128-n){pal_bad(2);return;}
 h[0]=128+n;h[1]=++pal_rows;h[3]=pal_owner;h[4]=GetTickCount();h[5]=queue;h[6]=sequence;
 h[30]=n?mnm_cont_crc32c(data,n):0;
 if(!write(pal_file,h,128)||(n&&!write(pal_file,data,n))){pal_bad(3);return;}
 pal_bytes+=128+n;
}
static u32 pal_bucket(u32 p){u32 at=((p>>4)*2654435761u)&8191;for(u32 i=0;i<8192;++i){u32 id=pal_lookup[at];if(!id||pal_node[id-1].pointer==p)return at;at=(at+1)&8191;}pal_bad(15);return 8192;}
static struct PalNode* pal_find(u32 p){u32 at=pal_bucket(p);if(at==8192||!pal_lookup[at])return 0;struct PalNode* n=pal_node+pal_lookup[at]-1;return n->live?n:0;}
static HANDLE pal_profile_file;static u32 pal_profile_rows,pal_profile_bytes,pal_profile_installed;
static int pal_string(u8* out,u32 pointer){
 if(!pointer||!readable(pointer,256))return 0;zero(out,256);
 for(u32 i=0;i<256;++i){out[i]=*(u8*)(pointer+i);if(!out[i])return 1;}return 0;
}
static void pal_profile_return(const struct PalCall* c,u32 result,u32 error){
 if(!c->profile)return;
 if(!pal_profile_file){
  char name[260];path(name,"palette-profiles-",1,".bin");pal_profile_file=CreateFileA(name,0x40000000,1,0,1,0x80,0);
  if(pal_profile_file==(HANDLE)-1){pal_profile_file=0;pal_bad(17);return;}
  u32 header[16]={0};copy(header,"MNMPPR01",8);header[2]=1;header[3]=64;header[4]=0x40209ca7;header[5]=PAL_NODES*5;header[6]=1344;header[7]=32*1024*1024;
  if(!write(pal_profile_file,header,64)){pal_bad(18);return;}pal_profile_bytes=64;
 }
 if(pal_profile_rows>=PAL_NODES*5||result>=c->api_args[4]||!pal_string(c->profile+1024,c->api_args[3])){pal_bad(19);return;}
 u32 h[16]={0};h[0]=1344;h[1]=++pal_profile_rows;h[2]=c->parent;h[3]=c->count;h[4]=pal_owner;h[5]=GetTickCount();h[6]=queue;h[7]=sequence;h[8]=c->caller;h[9]=result;h[10]=c->api_args[4];h[11]=error;h[12]=mnm_cont_crc32c(c->profile,1280);
 if(!write(pal_profile_file,h,64)||!write(pal_profile_file,c->profile,1280)){pal_bad(18);return;}pal_profile_bytes+=1344;
}
static void pal_profile_close(void){
 if(pal_profile_installed){u32 old,unused;if(get((void*)0x5c507c)!=(u32)palette_enter_11||!VirtualProtect((void*)0x5c507c,4,4,&old))pal_bad(20);else{put((void*)0x5c507c,palette_trampolines[11]);if(!VirtualProtect((void*)0x5c507c,4,old,&unused))pal_bad(20);}pal_profile_installed=0;}
 if(!pal_profile_file)return;u32 h[16]={0};copy(h,"MNMPPEND",8);h[2]=1;h[3]=64;h[4]=pal_profile_rows;h[5]=pal_profile_bytes;h[6]=pal_loss;h[7]=pal_owner;
 write(pal_profile_file,h,64);CloseHandle(pal_profile_file);pal_profile_file=0;
}
static void pal_close(void){
 if(!pal_file)return;u32 error=GetLastError(),h[32]={0};
 pal_profile_close();
 h[0]=128;h[1]=++pal_rows;h[2]=6;h[3]=pal_owner;h[7]=pal_loss;h[8]=pal_nodes;h[9]=pal_bytes;h[10]=pal_bindings;
 for(u32 i=0;i<pal_nodes;++i)if(pal_node[i].live)++h[11];
 for(u32 i=0;i<32;++i)if(pal_call[i].sp)++h[12];
 write(pal_file,h,128);CloseHandle(pal_file);pal_file=0;SetLastError(error);
}
/* Return interception preserves original RET4 builders and cdecl free. */
static void pal_floating(u32* h,const u8* fx){h[16]=fx[0]|(u32)fx[1]<<8;h[17]=get(fx+24);h[18]=fx[2]|(u32)fx[3]<<8;h[19]=fx[4];}
void palette_enter_observe(u32* r,u32 tag,const u8* fx){
 u32 error=GetLastError();if(!pal_file||pal_loss){SetLastError(error);return;}
 u32* args=r+9;struct PalNode* node=tag==10?pal_find(args[1]):0;
 struct PalCall* parent=0;
 if(tag==11){
  if(GetCurrentThreadId()!=pal_owner){SetLastError(error);return;}
  for(u32 i=0;i<32;++i){struct PalCall* candidate=pal_call+i;if(candidate->sp>(u32)args&&(candidate->tag==4||candidate->tag==5||candidate->tag==7)&&(!parent||candidate->sp<parent->sp))parent=candidate;}
  if(!parent){SetLastError(error);return;}
 }
 if(tag==10&&!node){SetLastError(error);return;}
 if(!pal_owned()){SetLastError(error);return;}
 struct PalCall* c=0;for(u32 i=0;i<32;++i)if(!pal_call[i].sp){c=pal_call+i;break;}
 if(!c){pal_bad(4);SetLastError(error);return;}
 c->sp=(u32)args+4+(tag>=12?28:tag==11?24:tag==10?0:4);c->caller=args[0];c->tag=tag;c->count=r[6];c->slot=r[5];c->source=args[1];c->id=0;
 if(tag>=12){
  struct PalNode* n=pal_find(c->slot);u32 bytes=n?12+(n->mode<8?n->count-1:n->count/2)*256*(n->mode<8?2:4):0;
  if(!n||n->mode!=(tag==12?1u:9u)||n->count!=c->count||!readable(c->source,768)||!readable(c->slot,bytes))pal_bad(22);
  else{
   u8* input=HeapAlloc(producer_heap(),0,872+bytes);
   if(!input)pal_bad(9);
   else{c->id=n->id;copy(input,(void*)c->source,768);copy(input+768,(void*)0x5f18d0,80);copy(input+848,args+2,24);copy(input+872,(void*)c->slot,bytes);zero(input+872,4);
    u32 h[32]={0};h[2]=7;h[7]=n->revision;h[8]=n->id;h[9]=n->count;h[10]=get((void*)0x6e1f88);h[11]=bytes;h[13]=tag;h[14]=c->caller;pal_floating(h,fx);pal_emit(h,input,872+bytes);HeapFree(producer_heap(),0,input);
   }
  }
  args[0]=(u32)palette_leave;SetLastError(error);return;
 }
 if(tag==11){
  c->parent=parent->id;c->count=parent->tag;for(u32 i=0;i<6;++i)c->api_args[i]=args[i+1];
  c->profile=HeapAlloc(producer_heap(),0,1280);
  if(!c->profile||!c->parent||c->api_args[4]!=256||!pal_string(c->profile,c->api_args[0])||!pal_string(c->profile+256,c->api_args[1])||!pal_string(c->profile+512,c->api_args[2])||!pal_string(c->profile+768,c->api_args[5]))pal_bad(21);
  args[0]=(u32)palette_leave;SetLastError(error);return;
 }
 u32 h[32]={0};h[2]=tag==10?3:1;h[13]=tag;h[14]=c->caller;pal_floating(h,fx);
 if(tag==10){c->id=node->id;h[8]=node->id;pal_emit(h,0,0);}
 else{
  u32 count=c->count;
  if(pal_nodes==PAL_NODES||count<2||count>256||(count&(count-1))||!readable(c->source,768)||!readable(c->slot,4)||get((void*)c->slot))pal_bad(5);
  else{u8 input[848];copy(input,(void*)c->source,768);copy(input+768,(void*)0x5f18d0,80);h[9]=count;h[10]=get((void*)0x6e1f88);pal_emit(h,input,sizeof(input));c->id=pal_rows;}
 }
 args[0]=(u32)palette_leave;SetLastError(error);
}
void palette_leave_observe(u32* r,const u8* fx){
 u32 error=GetLastError(),sp=(u32)r+40;
 for(u32 i=0;i<32;++i)if(pal_call[i].sp==sp){
  struct PalCall c=pal_call[i];zero(pal_call+i,sizeof(c));r[9]=c.caller;
  if(c.tag==11){if(pal_owned()&&!pal_loss)pal_profile_return(&c,r[7],error);if(c.profile)HeapFree(producer_heap(),0,c.profile);SetLastError(error);return;}
  if(!pal_owned()||pal_loss)break;
  u32 h[32]={0};h[2]=c.tag==10?4:2;h[13]=c.tag;h[14]=c.caller;h[15]=r[7];pal_floating(h,fx);
  if(c.tag>=12){
   struct PalNode* n=pal_find(c.slot);u32 bytes=n?12+(n->mode<8?n->count-1:n->count/2)*256*(n->mode<8?2:4):0;
   if(!n||n->id!=c.id||!readable(c.slot,bytes)||!readable(c.source,768))pal_bad(23);
   else{u8* body=HeapAlloc(producer_heap(),0,bytes+768);if(!body)pal_bad(9);else{
    if(r[7])++n->revision;h[2]=8;h[7]=n->revision;h[8]=n->id;h[9]=n->count;h[10]=get((void*)0x6e1f88);h[11]=bytes;copy(body,(void*)c.slot,bytes);zero(body,4);copy(body+bytes,(void*)c.source,768);pal_emit(h,body,bytes+768);HeapFree(producer_heap(),0,body);
   }}
   SetLastError(error);return;
  }
  if(c.tag==10){struct PalNode* n=0;for(u32 k=0;k<pal_nodes;++k)if(pal_node[k].id==c.id)n=pal_node+k;if(!n||!n->live)pal_bad(6);else{n->live=0;h[8]=n->id;pal_emit(h,0,0);}}
  else if(!readable(c.slot,4))pal_bad(7);
  else{
   u32 p=get((void*)c.slot),tables=c.tag<8?c.count-1:c.count/2,n=tables*256*(c.tag<8?2:4)+12;
   if(!r[7]||!p){h[9]=c.count;h[8]=0;pal_emit(h,0,0);}
   else if(pal_find(p)||!readable(p,n)||!readable(c.source,768)||get((void*)p))pal_bad(8);
   else{
    u8* body=HeapAlloc(producer_heap(),0,n+768);
    if(!body)pal_bad(9);
    else{copy(body,(void*)p,n);copy(body+n,(void*)c.source,768);
     struct PalNode* node=pal_node+pal_nodes++;node->pointer=p;node->id=pal_nodes;node->mode=c.tag;node->count=c.count;node->live=1;
     u32 bucket=pal_bucket(p);if(bucket<8192)pal_lookup[bucket]=node->id;
     h[8]=node->id;h[9]=c.count;h[10]=get((void*)0x6e1f88);h[11]=n;pal_emit(h,body,n+768);HeapFree(producer_heap(),0,body);
    }
   }
  }
  SetLastError(error);return;
 }
 SetLastError(error);
}
/* All selected states are checked; unchanged choices are deduplicated. A row
 * joins the prospective original request by queue/caller/geometry/frame CRC.
 * A missing tracked allocation is a loss, never a guessed identity. */
static void pal_bind(u32 frame,u32 head,u32 node,u32 ordinal,u32 table,const u32* fields,const u8* body){
 if(!pal_file||pal_loss||!pal_owned())return;
 struct PalNode* selected=pal_find(node);struct PalNode* root=pal_find(head);
 if(!selected||!root){pal_bad(10);return;}
 u32 chain[38],cursor=head;
 if(ordinal>18){pal_bad(12);return;}
 for(u32 i=0;i<=ordinal;++i){struct PalNode* link=pal_find(cursor);if(!link||!readable(cursor,12)){pal_bad(13);return;}chain[2*i]=link->id;chain[2*i+1]=link->revision;if(i<ordinal)cursor=get((void*)cursor);}
 if(cursor!=node){pal_bad(14);return;}
 u32 crc=mnm_cont_crc32c(body+fields[19],512),slot=((frame>>4)*2654435761u)&(PAL_FRAMES-1),k=0;
 while(pal_choice[slot].frame&&pal_choice[slot].frame!=frame&&k++<PAL_FRAMES)slot=(slot+1)&(PAL_FRAMES-1);
 if(k>=PAL_FRAMES){pal_bad(11);return;}
 struct PalChoice* c=pal_choice+slot;++pal_bindings;
 u32 value_at=(((frame>>4)*2654435761u)^(selected->id*2246822519u)^(table*3266489917u))&((PAL_FRAMES*4)-1),probes=0;
 while(pal_value[value_at].frame&&(pal_value[value_at].frame!=frame||pal_value[value_at].node!=selected->id||pal_value[value_at].table!=table)&&probes++<PAL_FRAMES*4)value_at=(value_at+1)&((PAL_FRAMES*4)-1);
 if(probes>=PAL_FRAMES*4){pal_bad(16);return;}
 struct PalValue* value=pal_value+value_at;
 int changed=!value->frame||value->crc!=crc||!c->frame||c->node!=selected->id||c->ordinal!=ordinal||c->revision!=selected->revision||value->revision!=selected->revision;
 value->frame=frame;value->node=selected->id;value->table=table;value->crc=crc;value->revision=selected->revision;
 c->frame=frame;c->node=selected->id;c->ordinal=ordinal;c->table=table;c->crc=crc;c->revision=selected->revision;
 if(!changed&&!(pal_full_prefix&&queue&&queue<=pal_full_prefix))return;
 u32 h[32]={0};h[2]=5;h[7]=selected->revision;h[8]=selected->id;h[9]=root->id;h[10]=ordinal;h[11]=table;h[12]=crc;
 h[13]=fields[15];h[14]=fields[22];h[15]=mnm_cont_crc32c(body,fields[19]);
 for(u32 i=0;i<8;++i)h[16+i]=fields[8+i];h[24]=fields[16];h[25]=fields[19];h[26]=fields[3];
 h[27]=ordinal+1;h[28]=get((void*)(node+4));h[29]=get((void*)(node+8));pal_emit(h,chain,(ordinal+1)*8);
}
static int pal_install(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_PALETTE_OBSERVE",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1')return 0;
 SetLastError(0);n=GetEnvironmentVariableA("MNM_PALETTE_FULL_PREFIX",flag,2);if(n){if(n!=1||flag[0]!='4')return 0;pal_full_prefix=4;}else if(GetLastError()!=203)return 0;
 const u32 addresses[14]={0x582440,0x582a80,0x583330,0x583bb0,0x586660,0x5870d0,0x585d80,0x587e70,0x588730,0x588bf0,0x5979b0,0,0x584430,0x589280};
 void(*entries[14])(void)={palette_enter_0,palette_enter_1,palette_enter_2,palette_enter_3,palette_enter_4,palette_enter_5,palette_enter_6,palette_enter_7,palette_enter_8,palette_enter_9,palette_enter_10,0,palette_enter_12,palette_enter_13};
 const u8 prologue[6]={0x55,0x8b,0xec,0x83,0xe4,0xf8},free_bytes[6]={0x8b,0x44,0x24,0x04,0x85,0xc0};
 for(u32 i=0;i<14;++i){if(i==11)continue;const u8* expected=i==10?free_bytes:prologue;if(!readable(addresses[i],6))return 0;for(u32 j=0;j<6;++j)if(*(u8*)(addresses[i]+j)!=expected[j])return 0;}
 void* kernel=GetModuleHandleA("KERNEL32.dll");u32 api=kernel?(u32)GetProcAddress(kernel,"GetPrivateProfileStringA"):0;
 if(!api||!readable(0x5c507c,4)||get((void*)0x5c507c)!=api)return 0;palette_trampolines[11]=api;
 char name[260];path(name,"palette-observation-",1,".bin");pal_file=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(pal_file==(HANDLE)-1){pal_file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMPAL03",8);h[2]=3;h[3]=64;h[4]=0x40209ca7;h[5]=PAL_BYTES;h[6]=PAL_NODES;h[7]=PAL_FRAMES;h[8]=pal_full_prefix;
 if(!write(pal_file,h,64))return 0;pal_bytes=64;
 for(u32 i=0;i<14;++i){if(i==11)continue;u8* t=VirtualAlloc(0,11,0x3000,0x40);if(!t)return 0;copy(t,(void*)addresses[i],6);t[6]=0xe9;put(t+7,addresses[i]+6-(u32)t-11);palette_trampolines[i]=(u32)t;}
 for(u32 i=0;i<14;++i){if(i==11)continue;u32 old,unused;if(!VirtualProtect((void*)addresses[i],6,0x40,&old))return 0;u8 patch[6]={0xe9,0,0,0,0,0x90};put(patch+1,(u32)entries[i]-addresses[i]-5);copy((void*)addresses[i],patch,6);if(!FlushInstructionCache(GetCurrentProcess(),(void*)addresses[i],6)||!VirtualProtect((void*)addresses[i],6,old,&unused))return 0;}
 {u32 old,unused;if(!VirtualProtect((void*)0x5c507c,4,4,&old))return 0;if(!__sync_bool_compare_and_swap((u32*)0x5c507c,api,(u32)palette_enter_11))return 0;pal_profile_installed=1;if(!VirtualProtect((void*)0x5c507c,4,old,&unused))return 0;}
 return 1;
}
