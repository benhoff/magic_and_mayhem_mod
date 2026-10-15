/* Process-local CPU authority. Identities contain no cross-process pointers.
 * A refusal terminates before original forwarding can read the same stale bytes. */
static u32 ns_history_refusals;
static void ns_cpu_require(struct NsState* s){
 if(!s->history_stale)return;
 ++ns_history_refusals;
 u32 record[8]={0x484d4e4d,1,(u32)s->pixels,s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1],ns_history_refusals},done=0;
 HANDLE file=CreateFileA("native-cpu-history-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,record,sizeof(record),&done,0);CloseHandle(file);}
 ExitProcess(124);
}
/* action0 requires current history (unknown non-owned memory is unaffected).
 * action1 invalidates an exact owned base with a nonzero generation and nonce.
 * action2 is a nonfatal diagnostic query, never permission to read stale pixels. */
__declspec(dllexport) i32 WIN NativeDrawCpuHistoryV1(const void* pixels,u32 action,u32 gl,u32 gh,u32 nl,u32 nh){
 for(struct NsState* s=ns_owned;s;s=s->next_owned){
  const u32 at=(u32)pixels,base=(u32)s->pixels;
  if(at<base || at-base>=s->pitch*s->height)continue;
  if(action==0){ns_cpu_require(s);return 0;}
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
