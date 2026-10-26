/* Process-local CPU authority. Identities contain no cross-process pointers.
 * A refusal terminates before original forwarding can read the same stale bytes. */
static u32 ns_history_refusals;
typedef i32 (WIN *NsCpuReader)(void*,const void*,u32,u32,u32);
static NsCpuReader ns_cpu_reader;
/* A pinned bridge registers outside any reader/fault. Never resolve the loader
 * or invoke this service from a vectored exception callback. */
__declspec(dllexport) i32 WIN NativeDrawCpuReaderV1(NsCpuReader reader){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!reader || (ns_cpu_reader && ns_cpu_reader!=reader))return ND_INVALID;
 ns_cpu_reader=reader;ns_cpu_reader_available=1;return 0;
}
static void ns_cpu_require_at(struct NsState* s,u32 family,u32 route,u32 caller){
 if(!s->history_stale)return;
 if(ns_cpu_reader){
  u32 error=GetLastError();i32 restored=ns_cpu_reader(&s->aliases[s->device->kind==3?3:1],s->pixels,family,route,caller);
  SetLastError(error);
  if(restored==1 && !s->history_stale)return;
 }
 ++ns_history_refusals;
 u32 record[11]={0x484d4e4d,2,(u32)s->pixels,s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1],ns_history_refusals,family,route,caller},done=0;
 HANDLE file=CreateFileA("native-cpu-history-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,record,sizeof(record),&done,0);CloseHandle(file);}
 ExitProcess(124);
}
static void ns_cpu_require(struct NsState* s){ns_cpu_require_at(s,0,0,(u32)__builtin_return_address(0));}
/* Read a current owned CPU result into private bridge storage. No COM Lock,
 * GPU wait or public mapping is introduced while its tracker is held. */
__declspec(dllexport) i32 WIN NativeDrawCpuSnapshotV1(void* object,u32* layout,u8* out,u32 bytes){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 struct NsState* s=0;
 for(struct NsState* p=ns_owned;p;p=p->next_owned)for(u32 i=0;i<5;++i)if(object==&p->aliases[i])s=p;
 if(!s || !layout)return ND_INVALID;
 if(ns_borrowed(s) || s->history_stale || s->cpu_guard_world)return NS_BUSY;
 u32 current[8]={(u32)s->pixels,s->width,s->height,s->pitch,s->bits,s->masks[0],s->masks[1],s->masks[2]};
 if(!out){if(bytes)return ND_INVALID;for(u32 i=0;i<8;++i)layout[i]=current[i];return 0;}
 if(bytes!=s->width*s->height*(s->bits/8) || (u32)out>0xffffffffu-bytes)return ND_INVALID;
 for(u32 i=0;i<8;++i)if(layout[i]!=current[i])return ND_INVALID;
 for(struct NsState* p=ns_owned;p;p=p->next_owned){u32 at=(u32)out,base=(u32)p->pixels,alias=(u32)p->pixels_alias;
  if((at<base+ns_guard_bytes(p) && base<at+bytes) || (alias && at<alias+ns_guard_bytes(p) && alias<at+bytes))return ND_INVALID;}
 const u32 row=s->width*(s->bits/8);const u8* source=(const u8*)(s->pixels_alias?s->pixels_alias:s->pixels);
 for(u32 y=0;y<s->height;++y)for(u32 x=0;x<row;++x)out[y*row+x]=source[y*s->pitch+x];
 return 0;
}
/* action0 requires current history (unknown non-owned memory is unaffected).
 * action1 invalidates an exact owned base with a nonzero generation and nonce.
 * action2 is a nonfatal diagnostic query, never permission to read stale pixels. */
__declspec(dllexport) i32 WIN NativeDrawCpuHistoryV1(const void* pixels,u32 action,u32 gl,u32 gh,u32 nl,u32 nh){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(action==3 && ns_guard_enabled<2)return 0;
 for(struct NsState* s=ns_owned;s;s=s->next_owned){
  const u32 at=(u32)pixels,base=(u32)s->pixels;
  if(at<base || at-base>=s->pitch*s->height)continue;
  if(action==0){ns_cpu_require_at(s,gl,gh,nl);return 0;}
  if(action==2)return s->history_stale?NS_BUSY:0;
  /* Census metadata only: mapped extent under a protected active Lock.
   * A positive answer never admits a load; actual instructions still fault. */
  if(action==3)return ns_guard_enabled>=2 && s->cpu_guard && s->locked>0 && !s->dc && !s->storage_dc &&
   gl && gl<=s->pitch*s->height-(at-base) && !gh && !nl && !nh;
  if(action!=1 || pixels!=s->pixels || (!gl&&!gh) || (!nl&&!nh))return ND_INVALID;
  /* A copied DC was admitted against current CPU history. It cannot become
   * stale behind its caller while that physical/logical lease remains live. */
  if(s->dc || s->storage_dc)return NS_BUSY;
  if(s->pixels_alias && s->cpu_guard_world && !s->cpu_guard_world_ready)return NS_BUSY;
  if((s->history_nonce[0]||s->history_nonce[1]) &&
     (s->history_nonce[0]!=nl || s->history_nonce[1]!=nh || gh<s->history_generation[1] ||
      (gh==s->history_generation[1] && gl<=s->history_generation[0])))return ND_INVALID;
  s->history_generation[0]=gl;s->history_generation[1]=gh;
  ns_guard_dirty_clear(s);
  s->history_nonce[0]=nl;s->history_nonce[1]=nh;s->history_stale=1;s->cpu_guard_world=s->cpu_guard_world_ready=0;ns_guard_refresh(s);return 0;
 }
 return action==0 || action==3?0:ND_INVALID;
}

/* Exact full/partial writable lease restore. The additional tuple fields prevent a
 * reply from surviving Unlock/re-Lock, a physical Flip or a foreign owner.
 * The bridge stages pending bytes, merges the native reply and publishes the
 * resulting complete WORDs before restoring these public pixels. */
__declspec(dllexport) i32 WIN NativeDrawCpuLeaseTransferV1(void* object,u32* identity,const u16* pixels,u32 bytes){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 struct NsState* s=0;
 for(struct NsState* p=ns_owned;p;p=p->next_owned)for(u32 i=0;i<5;++i)if(object==&p->aliases[i])s=p;
 if(!s || !identity || s->bits!=16 || s->masks[0]!=0xf800 || s->masks[1]!=0x7e0 || s->masks[2]!=31)return ND_INVALID;
 if(!s->pixels_alias || !s->cpu_dirty || s->locked!=1 || s->lock_owner!=GetCurrentThreadId() || s->dc || s->storage_dc || s->cpu_guard_world)return NS_BUSY;
 u32 current[12]={(u32)s->pixels,s->width,s->height,s->pitch,s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1],s->lock_owner,s->lock_serial,(u32)s->locked,s->lock_full};
 if(!pixels){if(bytes)return ND_INVALID;for(u32 i=0;i<12;++i)identity[i]=current[i];return s->history_stale?0:1;}
 if(!s->history_stale || bytes!=s->width*s->height*2)return ND_INVALID;
 for(u32 i=0;i<12;++i)if(identity[i]!=current[i])return ND_INVALID;
 if(!s->pixels_alias)ns_guard_set(s,0);
 u16* target=s->pixels_alias?s->pixels_alias:s->pixels;
 ns_copy_word_rows(target,s->pitch/2,pixels,s->width,s->width,s->height);
 s->history_stale=0;ns_guard_dirty_clear(s);ns_guard_refresh(s);return 0;
}
/* Process-local identity includes the physical base; it never crosses the wire.
 * Query returns1 for current CPU history,0 for a required native transfer.
 * Restore accepts only the same unborrowed physical history and tight RGB565. */
__declspec(dllexport) i32 WIN NativeDrawCpuTransferV1(void* object,u32* identity,const u16* pixels,u32 bytes){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 struct NsState* s=0;
 for(struct NsState* p=ns_owned;p;p=p->next_owned)for(u32 i=0;i<5;++i)if(object==&p->aliases[i])s=p;
 if(!s || !identity || s->bits!=16 || s->masks[0]!=0xf800 || s->masks[1]!=0x7e0 || s->masks[2]!=31)return ND_INVALID;
 if(ns_borrowed(s))return NS_BUSY;
 u32 current[8]={(u32)s->pixels,s->width,s->height,s->pitch,s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1]};
 if(!pixels){if(bytes)return ND_INVALID;for(u32 i=0;i<8;++i)identity[i]=current[i];return s->history_stale?0:1;}
 if(!s->history_stale || bytes!=s->width*s->height*2)return ND_INVALID;
 for(u32 i=0;i<8;++i)if(identity[i]!=current[i])return ND_INVALID;
 if(!s->pixels_alias)ns_guard_set(s,0);
 u16* target=s->pixels_alias?s->pixels_alias:s->pixels;
 ns_copy_word_rows(target,s->pitch/2,pixels,s->width,s->width,s->height);
 s->history_stale=0;ns_guard_dirty_clear(s);ns_guard_refresh(s);return 0;
}

/* Private write-mask snapshot under the bridge's ordered copy/tracker lease.
 * These compatibility words are not current GPU pixels. This service never
 * establishes CPU authority, seeds a GPU image or admits an application reader.
 * Exact physical layout is checked before temporarily opening protected pages. */
__declspec(dllexport) i32 WIN NativeDrawCpuLeaseSnapshotV1(void* object,const u32* layout,u16* out,u32 bytes){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 struct NsState* s=0;
 for(struct NsState* p=ns_owned;p;p=p->next_owned)for(u32 i=0;i<5;++i)if(object==&p->aliases[i])s=p;
 if(!s || !layout || !out || s->bits!=16 || s->masks[0]!=0xf800 || s->masks[1]!=0x7e0 || s->masks[2]!=31)return ND_INVALID;
 if(s->locked<=0 || s->lock_owner!=GetCurrentThreadId() || s->dc || s->storage_dc)return NS_BUSY;
 if(layout[0]!=(u32)s->pixels || layout[1]!=s->width || layout[2]!=s->height || layout[3]!=s->pitch || bytes!=s->width*s->height*2)return ND_INVALID;
 if(s->cpu_guard_world && s->pixels_alias && !s->cpu_guard_world_ready)return NS_BUSY;
 if(s->cpu_guard_world && !s->pixels_alias){
  u32 at=(u32)s->pixels,end=at+ns_guard_bytes(s);
  while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28 || m[4]!=0x1000 || m[5]!=4 || m[0]+m[3]<=at)return NS_BUSY;at=m[0]+m[3];}
 }
 u32 at=(u32)out;if(at>0xffffffffu-bytes)return ND_INVALID;
 for(struct NsState* p=ns_owned;p;p=p->next_owned){u32 base=(u32)p->pixels;
  if(at<base+ns_guard_bytes(p) && base<at+bytes)return ND_INVALID;}
 ns_guard_baseline(s,out);return 0;
}

/* Hand the stale allocation to the existing World NOACCESS guard. The scene
 * owner must install its guard immediately and restore it before a newer bound
 * history generation returns ownership here. Unlock/Flip cannot escape a loan. */
__declspec(dllexport) i32 WIN NativeDrawCpuGuardWorldV1(const void* pixels){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(ns_guard_enabled!=2)return 0;
 for(struct NsState* s=ns_owned;s;s=s->next_owned)if(s->pixels==pixels){
  if(s->locked<=0 || s->dc || s->storage_dc || s->cpu_guard_world)return NS_BUSY;
  if(!s->history_stale)return 0;
  if(!s->cpu_guard)return ND_INVALID;
  ns_guard_set(s,0);s->cpu_guard_world=1;return 1;
 }
 return ND_INVALID;
}

/* V2 borrows an already protected alias-backed allocation without opening its
 * public view. The scene explicitly ends its interval before the private bind.
 * Legacy modes retain their V1 protection ownership protocol. */
__declspec(dllexport) i32 WIN NativeDrawCpuGuardWorldV2(const void* pixels,u32 begin){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(begin>1)return ND_INVALID;
 if(ns_guard_enabled!=3)return begin?NativeDrawCpuGuardWorldV1(pixels):0;
 for(struct NsState* s=ns_owned;s;s=s->next_owned)if(s->pixels==pixels){
  if(s->locked!=1 || s->lock_owner!=GetCurrentThreadId() || s->dc || s->storage_dc)return NS_BUSY;
  if(!begin){
   if(!s->cpu_guard_world || s->cpu_guard_world_ready)return NS_BUSY;
   s->cpu_guard_world_ready=1;return 0;
  }
  if(!s->history_stale)return 0;
  if(!s->cpu_guard || s->cpu_guard_world)return NS_BUSY;
  s->cpu_guard_world=1;s->cpu_guard_world_ready=0;return 1;
 }
 return ND_INVALID;
}

/* Byte-exact pending writes, including writes equal to a stale private baseline.
 * action0 copies one byte flag per tight image byte; action1 clears after FIFO
 * commit; action2 returns bit0=dirty and bit1=incomplete WORD writes. No public
 * pages are opened. Layout and active owner are checked on every operation. */
__declspec(dllexport) i32 WIN NativeDrawCpuLeaseMaskV1(void* object,const u32* layout,u8* out,u32 bytes,u32 action){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 struct NsState* s=0;
 for(struct NsState* p=ns_owned;p;p=p->next_owned)for(u32 i=0;i<5;++i)if(object==&p->aliases[i])s=p;
 if(!s || !layout || action>2)return ND_INVALID;
 if(!s->cpu_dirty)return 0;
 if(s->dc || s->storage_dc || (s->locked && s->lock_owner!=GetCurrentThreadId()) || s->cpu_guard_world)return NS_BUSY;
 if(layout[0]!=(u32)s->pixels || layout[1]!=s->width || layout[2]!=s->height || layout[3]!=s->pitch)return ND_INVALID;
 if(action){if(out || bytes)return ND_INVALID;if(action==2)return (s->cpu_dirty_count?1:0)|(s->cpu_dirty_partial?2:0);ns_guard_dirty_clear(s);return 1;}
 if(!out || bytes!=s->width*s->height*2 || (u32)out>0xffffffffu-bytes)return ND_INVALID;
 for(struct NsState* p=ns_owned;p;p=p->next_owned){u32 at=(u32)out,base=(u32)p->pixels,alias=(u32)p->pixels_alias;
  if((at<base+ns_guard_bytes(p) && base<at+bytes) || (alias && at<alias+ns_guard_bytes(p) && alias<at+bytes))return ND_INVALID;}
 ns_expand_dirty_mask(out,s->cpu_dirty,bytes);
 return 1;
}
