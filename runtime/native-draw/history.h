/* Process-local CPU authority. Identities contain no cross-process pointers.
 * A refusal terminates before original forwarding can read the same stale bytes. */
static u32 ns_history_refusals;
static void ns_cpu_require_at(struct NsState* s,u32 family,u32 route,u32 caller){
 if(!s->history_stale)return;
 ++ns_history_refusals;
 u32 record[11]={0x484d4e4d,2,(u32)s->pixels,s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1],ns_history_refusals,family,route,caller},done=0;
 HANDLE file=CreateFileA("native-cpu-history-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,record,sizeof(record),&done,0);CloseHandle(file);}
 ExitProcess(124);
}
static void ns_cpu_require(struct NsState* s){ns_cpu_require_at(s,0,0,(u32)__builtin_return_address(0));}
/* action0 requires current history (unknown non-owned memory is unaffected).
 * action1 invalidates an exact owned base with a nonzero generation and nonce.
 * action2 is a nonfatal diagnostic query, never permission to read stale pixels. */
__declspec(dllexport) i32 WIN NativeDrawCpuHistoryV1(const void* pixels,u32 action,u32 gl,u32 gh,u32 nl,u32 nh){
 for(struct NsState* s=ns_owned;s;s=s->next_owned){
  const u32 at=(u32)pixels,base=(u32)s->pixels;
  if(at<base || at-base>=s->pitch*s->height)continue;
  if(action==0){ns_cpu_require_at(s,gl,gh,nl);return 0;}
  if(action==2)return s->history_stale?NS_BUSY:0;
  if(action!=1 || pixels!=s->pixels || (!gl&&!gh) || (!nl&&!nh))return ND_INVALID;
  if((s->history_nonce[0]||s->history_nonce[1]) &&
     (s->history_nonce[0]!=nl || s->history_nonce[1]!=nh || gh<s->history_generation[1] ||
      (gh==s->history_generation[1] && gl<=s->history_generation[0])))return ND_INVALID;
  s->history_generation[0]=gl;s->history_generation[1]=gh;
  s->history_nonce[0]=nl;s->history_nonce[1]=nh;s->history_stale=1;return 0;
 }
 return action==0?0:ND_INVALID;
}
/* Process-local identity includes the physical base; it never crosses the wire.
 * Query returns1 for current CPU history,0 for a required native transfer.
 * Restore accepts only the same unborrowed physical history and tight RGB565. */
__declspec(dllexport) i32 WIN NativeDrawCpuTransferV1(void* object,u32* identity,const u16* pixels,u32 bytes){
 struct NsState* s=0;
 for(struct NsState* p=ns_owned;p;p=p->next_owned)for(u32 i=0;i<5;++i)if(object==&p->aliases[i])s=p;
 if(!s || !identity || s->bits!=16 || s->masks[0]!=0xf800 || s->masks[1]!=0x7e0 || s->masks[2]!=31)return ND_INVALID;
 if(ns_borrowed(s))return NS_BUSY;
 u32 current[8]={(u32)s->pixels,s->width,s->height,s->pitch,s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1]};
 if(!pixels){if(bytes)return ND_INVALID;for(u32 i=0;i<8;++i)identity[i]=current[i];return s->history_stale?0:1;}
 if(!s->history_stale || bytes!=s->width*s->height*2)return ND_INVALID;
 for(u32 i=0;i<8;++i)if(identity[i]!=current[i])return ND_INVALID;
 for(u32 y=0;y<s->height;++y)for(u32 x=0;x<s->width;++x)s->pixels[y*(s->pitch/2)+x]=pixels[y*s->width+x];
 s->history_stale=0;return 0;
}
