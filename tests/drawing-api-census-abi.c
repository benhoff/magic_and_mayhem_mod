/* Real CALL-site stubs/observer/patching against synthetic PE32 bodies only. */
#include "../runtime/drawing-api-census/observer.c"
extern void canvas_test_call(u32,u32,u32,u32,void*);
static u8 before[560] __attribute__((aligned(16))),after[560] __attribute__((aligned(16)));
static volatile u32 body_calls,seen_argument;
static u32 slots[10],routines[10];
#define BODY(name,pop) __attribute__((naked)) static void name(void){__asm__ volatile("incl _body_calls; movl 4(%esp),%eax; movl %eax,_seen_argument; pushl $2345; call _SetLastError@4; movl $0x88760000,%eax; pushl $0x247; popfl; ret $" #pop);}
BODY(body0,0) BODY(body4,4) BODY(body8,8) BODY(body12,12)
BODY(body16,16) BODY(body20,20) BODY(body24,24)
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static void identical(void){for(u32 i=0;i<560;++i)require(before[i]==after[i],20);}
static void reset(void){used=sequence=serial=lost=active=io_failed=0;zero(frames,sizeof(frames));}
static void put(u8* out,u32 value){*(u32*)out=value;}
static void make(u32 index,u32 pop,u32 callee,u32 style){
 u8* code=VirtualAlloc(0,256,0x3000,0x40);require(code!=0,1);routines[index]=(u32)code;slots[index]=callee;
 u32 at=0;code[at++]=0x55;code[at++]=0x89;code[at++]=0xe5;
 for(u32 i=pop/4;i>0;--i){code[at++]=0xff;code[at++]=0x75;code[at++]=(u8)(4+i*4);}
 if(style==1){code[at++]=0xb8;put(code+at,callee);at+=4;}
 if(style==2){code[at++]=0xba;put(code+at,(u32)(slots+index));at+=4;}
 struct ApiHook* h=api_hooks+index;h->address=(u32)code+at;h->reg=7;
 if(style==0){code[at++]=0xff;code[at++]=0x15;put(code+at,(u32)(slots+index));at+=4;h->length=6;h->mode=3;h->operand=(u32)(slots+index);}
 else if(style==1){code[at++]=0xff;code[at++]=0xd0;code[at++]=0x85;code[at++]=0xc0;code[at++]=0x75;code[at++]=2;code[at++]=0x90;code[at++]=0x90;h->length=6;h->mode=1;}
 else if(style==2){code[at++]=0xff;code[at++]=0x12;code[at++]=0x85;code[at++]=0xc0;code[at++]=0x90;h->length=5;h->mode=2;h->reg=5;}
 else {code[at++]=0xe8;put(code+at,callee-h->address-5);at+=4;h->length=5;h->mode=4;h->operand=callee;}
 code[at++]=0x89;code[at++]=0xec;code[at++]=0x5d;code[at++]=0xc2;code[at++]=(u8)pop;code[at++]=0;
 copy(h->signature,(void*)h->address,h->length);
 u32 call_length=style==0?6:style==3?5:2;
 copy(h->code,h->signature,call_length);h->reloc_count=0;
 if(style==3){put(h->code+1,callee-5);h->reloc[h->reloc_count++]=1;}
 h->code[call_length]=0xe9;h->after_offset=call_length+1;put(h->code+h->after_offset,0-call_length-5);h->reloc[h->reloc_count++]=h->after_offset;
 h->rest=call_length+5;u32 end=h->rest;
 if(style==1){h->code[end++]=0x85;h->code[end++]=0xc0;h->code[end++]=0x0f;h->code[end++]=0x85;put(h->code+end,h->address+8-end-4);h->reloc[h->reloc_count++]=end;end+=4;}
 if(style==2){h->code[end++]=0x85;h->code[end++]=0xc0;h->code[end++]=0x90;}
 h->code[end++]=0xe9;put(h->code+end,h->address+h->length-end-4);h->reloc[h->reloc_count++]=end;end+=4;h->code_length=end;
 require(FlushInstructionCache(GetCurrentProcess(),code,at),2);
 require(prepare_trampoline(index),3);
}
static void exercise(u32 tag,u32 pop){
 body_calls=0;seen_argument=0;SetLastError(1234);canvas_test_call(routines[tag],0x12340000,0xabcdef00,pop,before);
 require(body_calls==1&&GetLastError()==2345,4);require(same((void*)api_hooks[tag].address,api_hooks[tag].signature,api_hooks[tag].length),5);
 require(patch_hook(tag),6);reset();body_calls=0;SetLastError(1234);canvas_test_call(routines[tag],0x12340000,0xabcdef00,pop,after);
 identical();require(body_calls==1&&GetLastError()==2345&&(!pop||seen_argument==4),21);
 require(!lost&&!active&&serial==1&&sequence==2,22);
 u32* rows=(u32*)buffer;require(rows[1]==1&&rows[33]==2&&rows[2]==rows[34]&&rows[3]==tag&&rows[35]==tag,23);
 require(rows[12]==1234&&rows[44]==2345&&rows[42]==0x88760000,24);
 require(rows[39]-rows[7]==pop&&rows[27]==rows[59]&&rows[27]==rows[7],25);
 require(rows[4]==2&&rows[36]==2&&rows[26]==1&&rows[58]==0,26);
 require(rows[17]==1&&(!pop||rows[18]==4)&&!rows[49]&&same(rows+50,(u32[8]){0},32),27);
 require(rows[16]==slots[tag]&&rows[48]==slots[tag],28);
}
void start(void){
 static u32 host[16]={0};host[4]=2;host[5]=2;host[6]=1;control=host;
 u32 callees[10]={(u32)body0,(u32)body4,(u32)body8,(u32)body12,(u32)body16,(u32)body20,(u32)body24,(u32)body8,(u32)body8,(u32)body8};
 u32 pops[10]={0,4,8,12,16,20,24,8,8,8};
 for(u32 tag=0;tag<10;++tag){make(tag,pops[tag],callees[tag],tag<7?0:tag-6);exercise(tag,pops[tag]);}
 /* All failure paths retain fixed original continuation and actual result. */
 reset();for(u32 i=0;i<2048;++i)frames[i].live=1;active=2048;body_calls=0;
 SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&lost==1&&active==2048&&!serial&&!sequence&&GetLastError()==2345,30);
 reset();host[4]=3;body_calls=0;SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&lost==1&&!active&&!serial&&!sequence&&GetLastError()==2345,31);host[4]=4;
 reset();sequence=MNM_DRAW_API_MAX_RECORDS;body_calls=0;SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&lost==1&&active==1&&sequence==MNM_DRAW_API_MAX_RECORDS&&GetLastError()==2345,32);
 reset();io_failed=1;body_calls=0;SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&io_failed&&!serial&&!sequence&&GetLastError()==2345,33);
 /* Per-thread most-recent frame is required: no matching older tag on loss. */
 reset();u32 regs[17]={0};api_observe(regs,0,1);api_observe(regs,1,1);api_observe(regs,0,2);require(lost==1&&active==2&&serial==2&&sequence==2,34);
 reset();api_observe(regs,0,1);api_observe(regs,0,1);api_observe(regs,0,2);api_observe(regs,0,2);require(!lost&&!active&&serial==2&&sequence==4,35);
 reset();api_observe(regs,0,2);require(lost==1&&!active&&!serial&&!sequence,36);
 ExitProcess(0);
}
