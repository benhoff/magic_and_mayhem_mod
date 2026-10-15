/* Hook9 copies 58d320..58d329, including CALL58b660 at +4.
 * Only that exact relocated CALL return is mapped. Continuations stay intact. */
static u32 storage_caller_rva(u32 caller,u32 trampoline,u32* mapped){
 *mapped=0;
 if(trampoline&&trampoline<=0xfffffff6u&&caller==trampoline+9){*mapped=1;return 0x18d329;}
 return caller>=0x400000&&caller<0x5c0000?caller-0x400000:0;
}
