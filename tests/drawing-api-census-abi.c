/* Real CALL-site stubs/observer/patching against synthetic PE32 bodies only. */
#include "../runtime/drawing-api-census/observer.c"
extern void api_detail_test_call(u32,u32,u32,u32,void*,const u32*);
static void canvas_test_call(u32 address,u32 ecx,u32 edx,u32 pop,void* out){api_detail_test_call(address,ecx,edx,pop,out,0);}
static u8 before[560] __attribute__((aligned(16))),after[560] __attribute__((aligned(16)));
static volatile u32 body_calls,seen_argument;
static volatile u32 seen_entry_flags,seen_entry_ecx,seen_entry_edx,seen_entry_sp,seen_entry_error,seen_last_argument;
static volatile u32 seen_thirteen[13];
static u32 slots[26],routines[26];
static u32 foreign_pc,foreign_calls;
static i32 WIN foreign_exception(struct ApiExceptionPointers* info){
 if(info->record->code!=0x80000003u||(u32)info->record->address!=foreign_pc)return 0;
 ++foreign_calls;info->context->eip=foreign_pc+1;return -1;
}
#define BODY(name,pop) __attribute__((naked)) static void name(void){__asm__ volatile("pushfl; popl _seen_entry_flags; movl %ecx,_seen_entry_ecx; movl %edx,_seen_entry_edx; movl %esp,_seen_entry_sp; .if " #pop "==52; .set mnm_abi_arg_offset,4; .rept 13; movl mnm_abi_arg_offset(%esp),%eax; movl %eax,_seen_thirteen+mnm_abi_arg_offset-4; .set mnm_abi_arg_offset,mnm_abi_arg_offset+4; .endr; .endif; call _GetLastError@0; movl %eax,_seen_entry_error; incl _body_calls; movl 52(%esp),%eax; movl %eax,_seen_last_argument; movl 4(%esp),%eax; movl %eax,_seen_argument; pushl $2345; call _SetLastError@4; movl $0x88760000,%eax; pushl $0x247; popfl; ret $" #pop);}
BODY(body0,0) BODY(body4,4) BODY(body8,8) BODY(body12,12)
BODY(body16,16) BODY(body20,20) BODY(body24,24) BODY(body52,52)
__attribute__((naked)) static void body_output(void){__asm__ volatile("incl _body_calls; movl 4(%esp),%eax; movl $0xaabbccdd,(%eax); movl $0x11223344,4(%eax); pushl $2345; call _SetLastError@4; movl $0x88760000,%eax; pushl $0x247; popfl; ret $24");}
static u32 candidate_old __attribute__((used));static u32* candidate_object;static u32 candidate_args[16];
__attribute__((naked)) static void body_candidate(void){__asm__ volatile("incl _body_calls; movl 4(%esp),%eax; pushl $_candidate_old; pushl $1; pushl $4096; pushl %eax; call _VirtualProtect@16; pushl $2345; call _SetLastError@4; movl $0x88760000,%eax; pushl $0x247; popfl; ret $8");}
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 current_case;
static void identical(void){for(u32 i=0;i<560;++i)if(before[i]!=after[i]){
 char directory[220],name[260];u32 n=GetEnvironmentVariableA("MNM_API_ABI_DIR",directory,sizeof(directory));
 if(n&&n<sizeof(directory)&&path(name,directory,"\\state-mismatch.bin")){
  HANDLE file=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(file!=(HANDLE)-1){write_binding(file,&current_case,4);write_binding(file,before,560);write_binding(file,after,560);CloseHandle(file);}
 }ExitProcess(20);
}}
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
 h->formal_words=pop/4;h->patch_bytes=(index>=10&&index<20)||index==21||index==23||index==25?1:h->length;
 if(h->patch_bytes==1)h->length=call_length;
 copy(h->code,h->signature,call_length);h->reloc_count=0;
 if(style==3){put(h->code+1,callee-5);h->reloc[h->reloc_count++]=1;}
 h->code[call_length]=0xe9;h->after_offset=call_length+1;put(h->code+h->after_offset,0-call_length-5);h->reloc[h->reloc_count++]=h->after_offset;
 h->rest=call_length+5;u32 end=h->rest;
 if(style==1&&h->patch_bytes!=1){h->code[end++]=0x85;h->code[end++]=0xc0;h->code[end++]=0x0f;h->code[end++]=0x85;put(h->code+end,h->address+8-end-4);h->reloc[h->reloc_count++]=end;end+=4;}
 if(style==2&&h->patch_bytes!=1){h->code[end++]=0x85;h->code[end++]=0xc0;h->code[end++]=0x90;}
 h->code[end++]=0xe9;put(h->code+end,h->address+h->length-end-4);h->reloc[h->reloc_count++]=end;end+=4;h->code_length=end;
 require(FlushInstructionCache(GetCurrentProcess(),code,at),2);
 require(prepare_trampoline(index),3);
}
static void exercise(u32 tag,u32 pop){
 current_case=tag;
 u8 original[32];copy(original,(void*)api_hooks[tag].address,32);
 body_calls=0;seen_argument=0;SetLastError(1234);canvas_test_call(routines[tag],0x12340000,0xabcdef00,pop,before);
 require(body_calls==1&&GetLastError()==2345,4);u32 entry[5]={seen_entry_flags,seen_entry_ecx,seen_entry_edx,seen_entry_sp,seen_entry_error};require(same((void*)api_hooks[tag].address,api_hooks[tag].signature,api_hooks[tag].length),5);
 require(patch_hook(tag),6);reset();body_calls=0;SetLastError(1234);canvas_test_call(routines[tag],0x12340000,0xabcdef00,pop,after);
 if(api_hooks[tag].patch_bytes==1){const struct ApiHook* h=api_hooks+tag;require(*(u8*)h->address==0xcc&&same((u8*)h->address+1,original+1,31),43);}
 identical();require(entry[0]==seen_entry_flags&&entry[1]==seen_entry_ecx&&entry[2]==seen_entry_edx&&entry[3]==seen_entry_sp&&seen_entry_error==1234,55);require(body_calls==1&&GetLastError()==2345&&(!pop||seen_argument==4),21);
 require(!lost&&!active&&serial==1&&sequence==2,22);
 u32* rows=(u32*)buffer;u32* post=rows+JOURNAL_RECORD/4;require(rows[1]==1&&post[1]==2&&rows[2]==post[2]&&rows[3]==tag&&post[3]==tag,23);
 require(rows[12]==1234&&post[12]==2345&&post[10]==0x88760000,24);
 require(post[7]-rows[7]==pop&&rows[27]==post[27]&&rows[27]==rows[7],25);
 require(rows[4]==2&&post[4]==2&&rows[26]==1&&post[26]==0,26);
 require(rows[17]==1&&(!pop||rows[18]==4)&&!post[17]&&same(post+18,(u32[8]){0},32),27);
 require(rows[16]==slots[tag]&&post[16]==slots[tag],28);
#ifndef MNM_COMPACT_CPU_JOURNAL
 require(rows[32]==pop/4&&rows[33]==pop/4&&rows[34]==1&&!post[34],56);
 for(u32 i=0;i<pop/4;++i)require(rows[36+i]==4+i,57);
 if(pop==52)require(rows[48]==16,58);
#endif
 if(pop==52)require(seen_last_argument==16,58);
 if(pop==52)for(u32 i=0;i<13;++i)require(seen_thirteen[i]==4+i,79);

}
static u32 output_area[2],output_args[16];
static void output_case(u32 tag){
 current_case=tag;
 make(tag,24,(u32)body_output,0);struct ApiHook* h=api_hooks+tag;h->snapshot_count=4;
 h->snapshots[0]=(struct ApiSnapshot){0,8,3};h->snapshots[1]=(struct ApiSnapshot){1,4,3};h->snapshots[2]=(struct ApiSnapshot){2,8,3};h->snapshots[3]=(struct ApiSnapshot){3,4,3};
 for(u32 i=0;i<16;++i)output_args[i]=4+i;output_args[0]=(u32)output_area;output_args[1]=0;output_args[2]=0xfffffff0u;output_args[3]=(u32)(output_area+1);
 output_area[0]=0x12345678;output_area[1]=0x55667788;SetLastError(1234);body_calls=0;api_detail_test_call(routines[tag],0x12340000,0xabcdef00,24,before,output_args);require(body_calls==1,59);
 require(patch_hook(tag),60);reset();output_area[0]=0x12345678;output_area[1]=0x55667788;SetLastError(1234);body_calls=0;api_detail_test_call(routines[tag],0x12340000,0xabcdef00,24,after,output_args);identical();require(body_calls==1&&!lost&&!active&&sequence==2&&serial==1&&GetLastError()==2345,61);
#ifndef MNM_COMPACT_CPU_JOURNAL
 u32* pre=(u32*)buffer;u32* post=pre+JOURNAL_RECORD/4;
 require(pre[56]==(u32)output_area&&post[56]==pre[56]&&pre[58]==1&&post[58]==1&&pre[60]==0x12345678&&post[60]==0xaabbccdd&&post[61]==0x11223344,62);
 require(pre[94]==2&&post[94]==2&&pre[130]==3&&post[130]==3,63);
 require(pre[36]==(u32)output_area&&!pre[37]&&pre[38]==0xfffffff0u&&post[10]==0x88760000,64);
#else
 u32* pre=(u32*)buffer;u32* post=pre+JOURNAL_RECORD/4;
 require(pre[17]==1&&pre[18]==(u32)output_area&&!pre[19]&&pre[20]==0xfffffff0u&&post[10]==0x88760000,64);
#endif
}
static void candidate_reset(u32 tag){
 u32 unused;require(VirtualProtect(candidate_object,4096,4,&unused),70);*candidate_object=(u32)(slots+tag);
 for(u32 i=0;i<16;++i)candidate_args[i]=4+i;candidate_args[0]=(u32)candidate_object;
}
static void candidate_case(u32 tag){
 current_case=tag;
 make(tag,8,(u32)body_candidate,2);candidate_reset(tag);SetLastError(1234);body_calls=0;api_detail_test_call(routines[tag],0x12340000,0xabcdef00,8,before,candidate_args);require(body_calls==1,71);
 require(patch_hook(tag),72);reset();candidate_reset(tag);SetLastError(1234);body_calls=0;api_detail_test_call(routines[tag],0x12340000,0xabcdef00,8,after,candidate_args);identical();require(body_calls==1&&!lost&&!active&&sequence==2&&serial==1&&GetLastError()==2345,73);
#ifndef MNM_COMPACT_CPU_JOURNAL
 u32* pre=(u32*)buffer;u32* post=pre+JOURNAL_RECORD/4;require(pre[52]==(u32)candidate_object&&pre[53]==(u32)(slots+tag)&&pre[54]==1&&same(pre+52,post+52,12),74);
#else
 require(((u32*)buffer)[17]==1&&((u32*)buffer)[18]==(u32)candidate_object,74);
#endif
 u32 memory[7];require(VirtualQuery(candidate_object,memory,28)==28&&memory[5]==1,75);
}
void start(void){
 static u32 host[16]={0};host[4]=2;host[5]=2;host[6]=1;control=host;
 u32 callees[10]={(u32)body0,(u32)body4,(u32)body8,(u32)body12,(u32)body16,(u32)body20,(u32)body24,(u32)body8,(u32)body8,(u32)body8};
 u32 pops[10]={0,4,8,12,16,20,24,8,8,8};
 require(AddVectoredExceptionHandler(0,foreign_exception)!=0,44);
 exception_handler=AddVectoredExceptionHandler(1,api_exception);require(exception_handler!=0,45);
 for(u32 tag=0;tag<20;++tag){u32 i=tag%10;make(tag,pops[i],callees[i],i<7?0:i-6);exercise(tag,pops[i]);}
 make(20,52,(u32)body52,0);exercise(20,52);make(21,52,(u32)body52,0);exercise(21,52);
 output_case(22);output_case(23);
 candidate_object=VirtualAlloc(0,4096,0x3000,4);require(candidate_object!=0,76);candidate_case(24);candidate_case(25);
 /* Unowned INT3 reaches the next handler. Observer mismatches never change
  * the supplied exception context or LastError. */
 u8* foreign=VirtualAlloc(0,16,0x3000,0x40);require(foreign!=0,46);foreign[0]=0xcc;foreign[1]=0xc3;foreign_pc=(u32)foreign;
 require(FlushInstructionCache(GetCurrentProcess(),foreign,2),47);SetLastError(1234);((void(*)(void))foreign)();require(foreign_calls==1&&GetLastError()==1234,48);
 struct ApiContextI386 context={0};struct ApiExceptionRecord record={0};struct ApiExceptionPointers pointers={&record,&context};
 record.code=0x80000003u;record.address=(void*)api_hooks[10].address;context.eip=api_hooks[10].address+2;
 SetLastError(1234);require(api_exception(&pointers)==0&&context.eip==api_hooks[10].address+2&&GetLastError()==1234,49);
 context.eip=api_hooks[10].address;record.code=0xc0000005u;require(api_exception(&pointers)==0&&context.eip==api_hooks[10].address,50);
 record.code=0x80000003u;record.address=(void*)foreign_pc;context.eip=foreign_pc;require(api_exception(&pointers)==0&&context.eip==foreign_pc,51);
 /* Execute an independent direct predecessor into a trapped CALL's untouched
  * successor. It bypasses the CALL and observer, as in the original graph. */
 u8* branch=VirtualAlloc(0,32,0x3000,0x40);require(branch!=0,52);branch[0]=0x55;branch[1]=0x89;branch[2]=0xe5;branch[3]=0xe9;
 put(branch+4,api_hooks[17].address+2-(u32)branch-8);require(FlushInstructionCache(GetCurrentProcess(),branch,8),53);
 reset();body_calls=0;SetLastError(1234);canvas_test_call((u32)branch,0x12340000,0xabcdef00,8,after);require(!body_calls&&!serial&&!sequence&&!lost&&!active&&GetLastError()==1234,54);
 /* All failure paths retain fixed original continuation and actual result. */
 current_case=99;
 /* Baseline and injected loss calls must share this caller frame. A saved
  * exercise() baseline has a different EBP/ESP when it is not inlined. */
 copy((void*)api_hooks[9].address,api_hooks[9].signature,api_hooks[9].length);
 require(FlushInstructionCache(GetCurrentProcess(),(void*)api_hooks[9].address,api_hooks[9].length),77);
 SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,before);
 require(patch_hook(9),78);
 reset();for(u32 i=0;i<2048;++i)frames[i].live=1;active=2048;body_calls=0;
 SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&lost==1&&active==2048&&!serial&&!sequence&&GetLastError()==2345,30);
 reset();host[4]=3;body_calls=0;SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&lost==1&&!active&&!serial&&!sequence&&GetLastError()==2345,31);host[4]=4;
 reset();sequence=JOURNAL_MAX_RECORDS;body_calls=0;SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&lost==1&&active==1&&sequence==JOURNAL_MAX_RECORDS&&GetLastError()==2345,32);
 reset();io_failed=1;body_calls=0;SetLastError(1234);canvas_test_call(routines[9],0x12340000,0xabcdef00,8,after);identical();require(body_calls==1&&io_failed&&!serial&&!sequence&&GetLastError()==2345,33);
 /* Per-thread most-recent frame is required: no matching older tag on loss. */
 reset();u32 regs[17]={0};api_observe(regs,0,1);api_observe(regs,1,1);api_observe(regs,0,2);require(lost==1&&active==2&&serial==2&&sequence==2,34);
 reset();api_observe(regs,0,1);api_observe(regs,0,1);api_observe(regs,0,2);api_observe(regs,0,2);require(!lost&&!active&&serial==2&&sequence==4,35);
 reset();api_observe(regs,0,2);require(lost==1&&!active&&!serial&&!sequence,36);
 /* Exercise the actual startup binding writer with this synthetic EXE's
  * module/stubs and prepared allocations. The test manifest is an explicit
  * 64-byte record, not a dumped native ApiHook object layout. */
 char directory[220],name[260];u32 n=GetEnvironmentVariableA("MNM_API_ABI_DIR",directory,sizeof(directory));require(n&&n<sizeof(directory),37);
 module_base=(u32)GetModuleHandleA(0);require(write_bindings(directory),38);
 require(path(name,directory,"\\abi-hooks.bin"),39);HANDLE file=CreateFileA(name,0x40000000,1,0,1,0x80,0);require(file!=(HANDLE)-1,40);
 for(u32 i=0;i<API_HOOKS;++i){const struct ApiHook* h=api_hooks+i;u32 row[32]={h->address,h->length,h->after_offset-1,h->mode,h->reg,h->operand};copy(row+6,h->signature,h->length);row[11]=h->patch_bytes;row[12]=h->formal_words;row[13]=h->snapshot_count;
  for(u32 n=0;n<h->snapshot_count;++n){row[16+n*4]=h->snapshots[n].argument;row[17+n*4]=h->snapshots[n].bytes;row[18+n*4]=h->snapshots[n].events;}
  require(write_binding(file,row,128),41);}
 require(CloseHandle(file),42);
 /* Serialize the actual native V2 writer's valid rows for strict decoder review.
  * Earlier intentional loss tests remain separate from this closed journal. */
 require(path(name,directory,"\\abi-api.bin"),65);journal=CreateFileA(name,0x40000000,1,0,1,0x80,0);require(journal!=(HANDLE)-1,66);reset();
 u32 header[16]={0};copy(header,JOURNAL_MAGIC,8);header[2]=JOURNAL_VERSION;header[3]=64;header[4]=JOURNAL_RECORD;header[5]=0x400000;header[6]=API_HOOKS;header[7]=JOURNAL_MAX_RECORDS;copy(header+8,original_sha256,32);require(write(header,64),67);
 for(u32 tag=0;tag<API_HOOKS;++tag){if(tag>=24){candidate_reset(tag);SetLastError(1234);api_detail_test_call(routines[tag],0x12340000,0xabcdef00,8,after,candidate_args);}else if(tag>=22){output_area[0]=0x12345678;output_area[1]=0x55667788;SetLastError(1234);api_detail_test_call(routines[tag],0x12340000,0xabcdef00,24,after,output_args);}else{SetLastError(1234);canvas_test_call(routines[tag],0x12340000,0xabcdef00,api_hooks[tag].formal_words*4,after);}}
 require(!lost&&!active&&!io_failed&&serial==API_HOOKS&&sequence==API_HOOKS*2,68);flush();u32 end[16]={0};copy(end,MNM_DRAW_API_END,8);end[2]=JOURNAL_VERSION;end[3]=sequence;end[4]=serial;require(write(end,64)&&CloseHandle(journal),69);journal=0;
 ExitProcess(0);
}
