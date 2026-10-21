/* Actual observer and exact worker fence with a nonmappable foreign source.
 * The provider enforces its cooperative producer lease. No hardware claim. */
static void provider_fixture_setup(void);
static void provider_fixture_check(void);
static void provider_fixture_worker(void);
#define MNM_BACKING_PROVIDER_FIXTURE
#define MNM_DIRECT_BACKING_FIXTURE
#define start private_fixture_start
#include "drawing-physical-owners-test.c"
#undef start
static u32 provider_kind,provider_fault,provider_live,provider_acquired,provider_verified,provider_released,provider_copied;
static u8 *provider_pixels,*provider_alias;
static HANDLE provider_mapping,provider_file;
static int provider_write(u8 value){if(provider_live)return 0;provider_alias[0]=value;return 1;}
static int provider_acquire(void* context,const MnmBackingRequestV2* q,MnmBackingReadV2* r){
 check(context==source&&q->version==2&&q->resource==source&&q->texture==source&&!q->mapped&&q->width==40&&q->height==40&&!q->pitch);
 check(!provider_live);provider_live=1;++provider_acquired;memset(r,0,sizeof(*r));
 r->version=2;r->bytes=sizeof(*r);r->kind=provider_kind;r->guarantees=7;r->object_low=100+provider_kind;r->generation=3;r->token=1;
 r->fence_low=9;r->fence_high=1;r->completed_low=9;r->completed_high=1;r->data=provider_pixels;r->pitch=80;r->size=3200;
 check(!provider_write(9));check(!DrawingUnregisterBackingV1(71));
 if(provider_fault==1)r->completed_low=8;
 if(provider_fault==2)r->guarantees=3;
 if(provider_fault==3)r->generation=0;
 if(provider_fault==4)r->kind=5;
 if(provider_fault==5){provider_live=0;--provider_acquired;return 0;}
 if(provider_fault==6)r->pitch=78;
 if(provider_fault==9)r->fence_low=r->fence_high=r->completed_low=r->completed_high=0;
 if(provider_fault==11)r->generation=2;
 if(provider_fault==12)r->generation=4;
 return 1;
}
static int provider_verify(void* context,const MnmBackingRequestV2* q,const MnmBackingReadV2* r){
 check(context==source&&q->resource==source&&r->token==1&&provider_live&&provider_copied==1);
 check(!provider_write(9)&&!DrawingUnregisterBackingV1(71));++provider_verified;
 check(!wine_phys_leases[2047].submission);
 if(provider_fault==7)wine_phys_writer(source,38);
 return provider_fault!=8&&!*(u32*)(source+0x54)&&!*(u32*)(source+0x58)&&*(u32*)(*(u32*)(source+0x120)+8)==77;
}
static void provider_release(void* context,const MnmBackingReadV2* r){check(context==source&&r->token==1&&provider_live);provider_live=0;++provider_released;}
static void provider_fixture_worker(void){
 check(provider_live&&!provider_released&&!provider_verified&&provider_acquired==1);
 check(!*(u32*)(source+0x54)&&!*(u32*)(source+0x58)&&!*(u32*)(*(u32*)(source+0x120)+0x30)&&*(u32*)(*(u32*)(source+0x120)+8)==77);
 check(!provider_write(9)&&!DrawingUnregisterBackingV1(71));
 u8* dest=(u8*)*(u32*)(destination+0x58);for(u32 i=0;i<3200;++i){check(provider_alias[i]==(u8)(3+i%251));dest[i]=provider_alias[i];}++provider_copied;
 if(provider_fault==13)provider_pixels[0]^=1;
}
static void provider_fixture_setup(void){
 wine_phys_free((void*)*(u32*)(source+0x54));*(u32*)(source+0x54)=*(u32*)(source+0x58)=0;
 provider_file=(HANDLE)-1;
 if(provider_kind==2){char name[600];path(name,"file-backing.bin");provider_file=CreateFileA(name,0xc0000000,3,0,1,0x80,0);check(provider_file!=(HANDLE)-1);}
 provider_mapping=CreateFileMappingA(provider_file,0,4,0,4096,0);check(provider_mapping!=0);
 provider_pixels=MapViewOfFile(provider_mapping,2,0,0,4096);provider_alias=MapViewOfFile(provider_mapping,2,0,0,4096);check(provider_pixels&&provider_alias&&provider_pixels!=provider_alias);
 for(u32 i=0;i<3200;++i)provider_pixels[i]=(u8)(3+i%251);
 *(u32*)(source+0x30)=1;u32* sub=(u32*)*(u32*)(source+0x120);sub[12]=0;sub[2]=77;
 struct MnmBackingProviderV2 api={2,sizeof(api),71,provider_kind,source,&provider_acquire,&provider_verify,&provider_release};
 struct MnmBackingProviderV2 invalid=api;invalid.version=1;check(!DrawingRegisterBackingV2(&invalid));invalid=api;invalid.kind=0;check(!DrawingRegisterBackingV2(&invalid));invalid=api;invalid.release=0;check(!DrawingRegisterBackingV2(&invalid));invalid=api;invalid.release=(void(*)(void*,const MnmBackingReadV2*))&provider_kind;check(!DrawingRegisterBackingV2(&invalid));
 check(!DrawingUnregisterBackingV1(999));
 if(wine_source_ownership_required==1){check(!DrawingRegisterBackingV2(&api));finish(1);ExitProcess(0);}
 check(DrawingRegisterBackingV2(&api));check(!DrawingRegisterBackingV2(&api));
 if(provider_fault==10)wine_backing_providers[0].last_token=1;
 if(provider_fault==11||provider_fault==12){struct WineBackingObject* o=wine_backing_objects;o->provider=71;o->low=100+provider_kind;o->generation=3;o->active=provider_fault==12;}
}
static void provider_fixture_check(void){
 check(!provider_live&&provider_acquired==1&&provider_verified==1&&provider_released==1&&provider_copied==1);
 const u8* dest=(void*)*(u32*)(destination+0x58);for(u32 i=0;i<3200;++i)check(dest[i]==(u8)(3+i%251));
 check(provider_write(4)&&provider_pixels[0]==4);check(DrawingUnregisterBackingV1(71));check(!DrawingUnregisterBackingV1(71));
}
void start(void){char value[4];check(GetEnvironmentVariableA("MNM_TEST_BACKING",value,4)==1);provider_kind=value[0]-'0';check(provider_kind>=1&&provider_kind<=4);if(GetEnvironmentVariableA("MNM_TEST_BACKING_FAULT",value,4)==1)provider_fault=value[0]-'0';private_fixture_start();}
