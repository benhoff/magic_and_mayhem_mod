/* Optional local shutdown evidence. Counts precede process reclamation: this
 * never releases a game reference or turns process exit into COM teardown.
 * Creator addresses are launch-local diagnostic values, not shared pointers. */
static void nd_retirement_write(HANDLE file,const u32* words,u32 count){
 u32 done=0;if(!WriteFile(file,words,count*4,&done,0) || done!=count*4)ExitProcess(126);
}
static void nd_retirement_report(u32 process_exit){
 static char path[4096];u32 error=GetLastError();
 u32 count=GetEnvironmentVariableA("MNM_NATIVE_DRAW_RETIREMENT",path,sizeof(path));
 if(!count){SetLastError(error);return;}if(count>=sizeof(path))ExitProcess(126);
 HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
 if(file==(HANDLE)-1)ExitProcess(126);
 u32 header[16]={0x444d4e4d,0x31544552,1,64,process_exit,nd_objects,nd_surfaces,ns_dcs,nc_objects,np_objects,
  ns_bytes,nd_creates,ns_creates,GetModuleHandleA("ddraw.dll")!=0,0,0};
 nd_retirement_write(file,header,16);
 for(struct NdState* s=nd_owned;s;s=s->next_owned){
  u32 row[16]={1,(u32)s,s->creator,s->interfaces,s->refs[0],s->refs[1],s->refs[2],s->refs[3],
   s->width,s->height,s->depth,s->coop,(u32)s->primary,0,0,0};
  nd_retirement_write(file,row,16);
 }
 for(struct NsState* s=ns_owned;s;s=s->next_owned){
  u32 row[16]={2,(u32)s,s->creator,s->interfaces,s->refs[0],s->refs[1],s->refs[2],s->refs[3],
   s->refs[4],s->width,s->height,s->caps,(u32)s->device->state,s->locked,s->dc!=0,s->storage_dc!=0};
  nd_retirement_write(file,row,16);
 }
 CloseHandle(file);SetLastError(error);
}
