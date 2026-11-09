/* Execute production i386 entry/return assembly against authored callees.
 * API callbacks deliberately clobber x87 and XMM0. No original instructions
 * beyond the declared authored ABI prologues serve as an equivalence oracle. */
#define PALETTE_ABI
#define main encoder_main
#include "palette-observation-fixture.c"
#undef main
extern void palette_abi_call(u32*,u32,u32,u32,u32);
extern void palette_abi_profile_call(u32*,u32,u32,u32,u32,u32,u32,u32);
void check_profile_call(const u32* args){
 u32 out[9]={0};long double value=3.25L,result;u8 vector[16],after[16];for(u32 i=0;i<16;++i)vector[i]=i+1;
 unsigned short before,now;u32 mx_before,mx_after;
 __asm__ volatile("fnstcw %0;fldt %2;movdqu %3,%%xmm0;stmxcsr %1":"=m"(before),"=m"(mx_before):"m"(value),"m"(vector):"memory");
 palette_abi_profile_call(out,get((void*)0x5c507c),args[0],args[1],args[2],args[3],args[4],args[5]);
 __asm__ volatile("fnstcw %0;fstpt %2;movdqu %%xmm0,%3;stmxcsr %1":"=m"(now),"=m"(mx_after),"=m"(result),"=m"(after)::"memory");
 if(out[0]!=0x71717171||out[1]!=0x61616161||out[4]!=0x51515151||out[5]!=0x42424242||out[6]!=4||out[7]!=3||out[3]!=out[2]-4||!(out[8]&0x400)||before!=now||mx_before!=mx_after||memcmp(&value,&result,10)||memcmp(vector,after,16)||GetLastError()!=123)exit(24);
 for(u32 i=0;i<6;++i)if(get((void*)(0x5f1000+i*4))!=args[i])exit(25);
}
static void check_call(u32 address,u32 slot,u32 source,u32 expected,u32 cleanup){
 u32 out[9]={0};long double value=3.25L,result;u8 vector[16],after[16];for(u32 i=0;i<16;++i)vector[i]=i+1;
 u32 mx_before,mx_after;unsigned short cw_before,cw_after;SetLastError(123);__asm__ volatile("fnstcw %0":"=m"(cw_before));
 __asm__ volatile("fldt %1;movdqu %2,%%xmm0;stmxcsr %0":"=m"(mx_before):"m"(value),"m"(vector):"memory");
 palette_abi_call(out,address,4,slot,source);
 __asm__ volatile("fstpt %0;movdqu %%xmm0,%1;stmxcsr %2;fnstcw %3":"=m"(result),"=m"(after),"=m"(mx_after),"=m"(cw_after)::"memory");
 if(out[0]!=0x71717171||out[1]!=0x61616161||out[4]!=0x51515151||out[5]!=slot||out[6]!=4||out[7]!=expected||out[3]!=out[2]-4-cleanup||!(out[8]&0x400)||GetLastError()!=123||memcmp(&value,&result,10)||memcmp(vector,after,16)||mx_before!=mx_after||cw_before!=cw_after)exit(20);
}
extern void palette_abi_refresh_call(u32*,u32,u32,const u32*);
static void check_refresh(u32 address,u32 node,u32 source){
 u32 out[9]={0},args[7]={source,0,0,0,0,0,0};long double value=3.25L,result;u8 vector[16],after[16];for(u32 i=0;i<16;++i)vector[i]=i+1;
 unsigned short before,now;u32 mx_before,mx_after;SetLastError(123);
 __asm__ volatile("fnstcw %0;fldt %2;movdqu %3,%%xmm0;stmxcsr %1":"=m"(before),"=m"(mx_before):"m"(value),"m"(vector):"memory");
 palette_abi_refresh_call(out,address,node,args);
 __asm__ volatile("fnstcw %0;fstpt %2;movdqu %%xmm0,%3;stmxcsr %1":"=m"(now),"=m"(mx_after),"=m"(result),"=m"(after)::"memory");
 if(out[0]!=0x71717171||out[1]!=0x61616161||out[4]!=0x51515151||out[5]!=node||out[6]!=4||out[7]!=0||out[3]!=out[2]-4||!(out[8]&0x400)||before!=now||mx_before!=mx_after||memcmp(&value,&result,10)||memcmp(vector,after,16)||GetLastError()!=123)exit(27);
}
int main(int argc,char** argv){
 if(argc!=2)return 2;destination=argv[1];
 void* globals=mmap((void*)0x580000,0x180000,7,MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,-1,0);if(globals==MAP_FAILED)return 3;registered(globals,0x180000);
 const u32 addresses[]={0x582440,0x582a80,0x583330,0x583bb0,0x586660,0x5870d0,0x585d80,0x587e70,0x588730,0x588bf0,0x5979b0,0,0x584430,0x589280};
 for(u32 i=0;i<10;++i)memcpy((void*)addresses[i],"\x55\x8b\xec\x83\xe4\xf8\x31\xc0\x89\xec\x5d\xc2\x04\x00",14);
 for(u32 i=12;i<14;++i)memcpy((void*)addresses[i],"\x55\x8b\xec\x83\xe4\xf8\x31\xc0\x89\xec\x5d\xc2\x1c\x00",14);
 memcpy((void*)addresses[10],"\x8b\x44\x24\x04\x85\xc0\xc3",7);
 u8* api=(u8*)0x5b0000;for(u32 i=0;i<6;++i){api[0]=0x8b;api[1]=0x44;api[2]=0x24;api[3]=(u8)(4+i*4);api[4]=0xa3;put(api+5,0x5f1000+i*4);api+=9;}
 const u8 tail[]={0x8b,0x44,0x24,0x10,0xc7,0x00,0x2e,0x32,0x35,0x00,0xb8,3,0,0,0,0xc2,0x18,0};copy(api,tail,sizeof(tail));put((void*)0x5c507c,0x5b0000);
 if(!pal_install())return 9;u32* slot=allocation(4);u8* rgb=allocation(768);
 unsigned short saved;__asm__ volatile("fnstcw %0":"=m"(saved));
 for(u32 precision=0;precision<2;++precision){const unsigned short cw=precision?0x27f:0x37f;__asm__ volatile("fldcw %0"::"m"(cw));for(u32 i=0;i<10;++i)check_call(addresses[i],(u32)slot,(u32)rgb,0,0);}
 __asm__ volatile("fldcw %0"::"m"(saved));
 u32* node=builder(0,100);
 check_call(addresses[10],0,(u32)node,(u32)node,4);
 u32* profileNode=builder(4,100);retire(profileNode);
 u32* wordNode=builder(1,100);u32* wideNode=builder(9,100);
 for(u32 precision=0;precision<2;++precision){const unsigned short cw=precision?0x27f:0x37f;__asm__ volatile("fldcw %0"::"m"(cw));check_refresh(0x584430,(u32)wordNode,(u32)rgb);check_refresh(0x589280,(u32)wideNode,(u32)rgb);}
 __asm__ volatile("fldcw %0"::"m"(saved));retire(wordNode);retire(wideNode);
 if(pal_loss)return 21;pal_close();if(get((void*)0x5c507c)!=0x5b0000)return 23;puts("28 production hook entry/return ABI cases passed");return 0;
}
