/* Pinned DirectX-check adapter, separate from normal COM Release semantics.
 * The two guarded original call sites lose their stack-only Surface aliases.
 * Prove that exact temporary graph before releasing any of its references. */
__declspec(dllexport) u32 WIN NativeDrawReleaseStartupProbeV1(void* pointer){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 const u32 error=GetLastError(),base=(u32)GetModuleHandleA(0);
 struct NdState* device=0;
 for(struct NdState* p=nd_owned;p;p=p->next_owned)if(pointer==&p->first){device=p;break;}
 u32 caller=(u32)__builtin_return_address(0)-base;
 if(!device || device->creator-base!=0xbd96d ||
    (caller!=0xbdb2a && caller!=0xbdb6f) ||
    device->refs[0] || device->refs[1]!=2 || device->refs[2] || device->refs[3] || device->interfaces!=1 ||
    device->coop!=8 || device->width!=800 || device->height!=600 || device->depth!=16)ExitProcess(127);
 struct NsState* surface=device->primary;
 if(!surface || surface->device!=&device->first || surface->creator-base!=0xbdadc || surface->caps!=0x200 ||
    surface->width!=800 || surface->height!=600 || surface->bits!=16 || surface->history_stale ||
    surface->back || surface->clipper || surface->palette || ns_borrowed(surface) ||
    surface->refs[0] || surface->refs[2] || surface->refs[1]>1 || surface->refs[3]>1 || surface->refs[4]>1)ExitProcess(127);
 for(struct NsState* p=ns_owned;p;p=p->next_owned)if(p!=surface && p->device->state==device)ExitProcess(127);
 u32 aliases[3]={surface->refs[1],surface->refs[3],surface->refs[4]},count=aliases[0]+aliases[1]+aliases[2];
 if(!count || count!=surface->interfaces)ExitProcess(127);
 /* Preserve the final Release result the original checker would have seen.
  * It is independent of the now-correct cleanup of its lost child aliases. */
 const u32 original_result=device->refs[1]-1,before_devices=nd_objects,before_surfaces=nd_surfaces;
 const u32 kinds[3]={1,3,4};
 for(u32 i=0;i<3;++i)if(aliases[i])ns_release(&surface->aliases[kinds[i]]);
 if(ns_lifetime_mode)ns_lifetime_collect();
 u32 released=nd_release(&device->first);
 if(released || nd_objects+1!=before_devices || nd_surfaces+1!=before_surfaces)ExitProcess(127);
 u32 receipt[16]={0x444d4e4d,0x31425250,1,64,caller,count,
  aliases[0],aliases[1],aliases[2],before_devices,nd_objects,before_surfaces,nd_surfaces,original_result,released,0},done=0;
 HANDLE file=CreateFileA("native-draw-startup-retirement.bin",0x40000000,1,0,1,0x80,0);
 if(file==(HANDLE)-1 || !WriteFile(file,receipt,sizeof(receipt),&done,0) || done!=sizeof(receipt))ExitProcess(127);
 CloseHandle(file);SetLastError(error);return original_result;
}
