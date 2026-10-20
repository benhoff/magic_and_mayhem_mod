/* Full actual observer COPY -> private stage -> worker/fence -> restore.
 * Foreign producer enforcement and GPU completion are controlled dependencies;
 * shared/file mappings use real two-view Windows section objects. */
static void provider_fixture_setup(void);
static void provider_fixture_check(void);
#define MNM_BACKING_PROVIDER_FIXTURE
#define start private_fixture_start
#include "drawing-physical-owners-test.c"
#undef start
static u32 provider_kind,provider_fault,provider_live,provider_acquired,provider_bound,provider_restored,provider_released;
static u8 *provider_pixels,*provider_alias;
static u32 provider_flags,provider_bo;
static HANDLE provider_mapping,provider_file;
static int provider_write(u8 value){if(provider_live)return 0;provider_alias[0]=value;return 1;}
static int provider_acquire(void* context,const struct MnmBackingRequestV1* q,struct MnmBackingReadV1* r){
 check(context==source&&q->resource==source&&q->texture==source&&q->mapped==provider_pixels&&q->width==40&&q->height==40&&q->pitch==80);
 check(!provider_live);provider_live=1;++provider_acquired;memset(r,0,sizeof(*r));
 r->version=1;r->bytes=sizeof(*r);r->kind=provider_kind;r->guarantees=15;r->object_low=100+provider_kind;r->generation=3;r->token=1;
 r->fence_low=9;r->fence_high=1;r->completed_low=9;r->completed_high=1;r->data=provider_pixels;r->pitch=80;r->size=3200;
 check(!provider_write(9));check(!DrawingUnregisterBackingV1(71));
 if(provider_fault==1)r->completed_low=8;
 if(provider_fault==2)r->guarantees=7;
 if(provider_fault==3)r->generation=0;
 if(provider_fault==4)r->kind=5;
 if(provider_fault==5){provider_live=0;--provider_acquired;return 0;}
 if(provider_fault==6)r->pitch=78;
 if(provider_fault==11)r->generation=2;
 if(provider_fault==12)r->generation=4;
 if(provider_fault==9){r->fence_low=r->fence_high=r->completed_low=r->completed_high=0;}
 return 1;
}
static int provider_bind(void* context,const struct MnmBackingRequestV1* q,const struct MnmBackingReadV1* r,const void* pixels,u32 pitch){
 check(context==source&&q->resource==source&&r->token==1&&provider_live&&pitch==80);++provider_bound;
 wine_phys_writer(source,38);u32* sub=(u32*)*(u32*)(source+0x120);
 *(u32*)(source+0x30)=0;*(u32*)(source+0x54)=*(u32*)(source+0x58)=0;sub[2]=0;sub[6]=2;sub[12]=(u32)pixels;*(u32*)(source+0xc4)&=~0x50u;
 if(provider_fault==7)*(u32*)(source+0x30)=1;
 return 1;
}
static int provider_restore(void* context,const struct MnmBackingRequestV1* q,const struct MnmBackingReadV1* r){
 check(context==source&&q->resource==source&&r->token==1&&provider_live&&provider_bound==1);++provider_restored;
 check(!provider_write(9)&&!DrawingUnregisterBackingV1(71));wine_phys_writer(source,38);
 u32* sub=(u32*)*(u32*)(source+0x120);*(u32*)(source+0x30)=provider_flags;*(u32*)(source+0x58)=(u32)provider_pixels;sub[12]=(u32)provider_pixels;sub[2]=provider_bo;
 return provider_fault!=8;
}
static int provider_verify(void* context,const struct MnmBackingRequestV1* q,const struct MnmBackingReadV1* r){
 check(context==source&&q->resource==source&&r->token==1&&provider_live&&provider_restored==1);
 return *(u32*)(source+0x30)==provider_flags&&*(u32*)(source+0x58)==(u32)provider_pixels&&*(u32*)(*(u32*)(source+0x120)+8)==provider_bo;
}
static void provider_release(void* context,const struct MnmBackingReadV1* r){
 check(context==source&&r->token==1&&provider_live);provider_live=0;++provider_released;
}
static void provider_fixture_setup(void){
 wine_phys_free((void*)*(u32*)(source+0x54));*(u32*)(source+0x54)=*(u32*)(source+0x58)=0;
 provider_file=(HANDLE)-1;
 if(provider_kind==2){char name[600];path(name,"file-backing.bin");provider_file=CreateFileA(name,0xc0000000,3,0,1,0x80,0);check(provider_file!=(HANDLE)-1);}
 provider_mapping=CreateFileMappingA(provider_file,0,4,0,4096,0);check(provider_mapping!=0);
 provider_pixels=MapViewOfFile(provider_mapping,2,0,0,4096);provider_alias=MapViewOfFile(provider_mapping,2,0,0,4096);check(provider_pixels&&provider_alias&&provider_pixels!=provider_alias);
 memset(provider_pixels,3,3200);check(provider_alias[0]==3);provider_flags=provider_kind==1?1:provider_kind==3?4:0;provider_bo=provider_kind==4?77:0;
 *(u32*)(source+0x30)=provider_flags;*(u32*)(source+0x58)=(u32)provider_pixels;u32* sub=(u32*)*(u32*)(source+0x120);sub[12]=(u32)provider_pixels;sub[2]=provider_bo;
 *(u32*)(*(u32*)(alias+0x120)+0x30)=(u32)provider_alias;initial_client=(u32)provider_pixels;old_pixels=provider_pixels;
 struct MnmBackingProviderV1 api={1,sizeof(api),71,provider_kind,source,&provider_acquire,&provider_bind,&provider_restore,&provider_verify,&provider_release};
 struct MnmBackingProviderV1 invalid=api;invalid.version=2;check(!DrawingRegisterBackingV1(&invalid));invalid=api;invalid.kind=0;check(!DrawingRegisterBackingV1(&invalid));invalid=api;invalid.release=0;check(!DrawingRegisterBackingV1(&invalid));invalid=api;invalid.release=(void(*)(void*,const struct MnmBackingReadV1*))&provider_kind;check(!DrawingRegisterBackingV1(&invalid));invalid=api;invalid.release=(void(*)(void*,const struct MnmBackingReadV1*))0x1234;check(!DrawingRegisterBackingV1(&invalid));
 check(!DrawingUnregisterBackingV1(999));check(DrawingRegisterBackingV1(&api));check(!DrawingRegisterBackingV1(&api));
 if(provider_fault==10)wine_backing_providers[0].last_token=1;
 if(provider_fault==11||provider_fault==12){struct WineBackingObject* o=wine_backing_objects;o->provider=71;o->low=100+provider_kind;o->generation=3;o->active=provider_fault==12;}
}
static void provider_fixture_check(void){
 check(!provider_live&&provider_acquired==1&&provider_bound==1&&provider_restored==1&&provider_released==1);
 check(provider_pixels[0]==3&&provider_alias[0]==3&&provider_write(4)&&provider_pixels[0]==4);
 check(DrawingUnregisterBackingV1(71));check(!DrawingUnregisterBackingV1(71));
}
void start(void){char value[4];check(GetEnvironmentVariableA("MNM_TEST_BACKING",value,4)==1);provider_kind=value[0]-'0';check(provider_kind>=1&&provider_kind<=4);
 if(GetEnvironmentVariableA("MNM_TEST_BACKING_FAULT",value,4)==1)provider_fault=value[0]-'0';
 private_fixture_start();
}
