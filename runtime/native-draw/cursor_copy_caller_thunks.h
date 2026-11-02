/* Opt-in observation only. Arguments/thiscall RET pop match the existing
 * original service calls. Publish the exact outgoing original return slot;
 * unique assembly labels cannot drift across compiler-generated epilogues. */
extern u8 nd_copy_caller_whole_return[],nd_copy_caller_rect_return[];
void WIN nd_copy_caller_whole(void*,void*,void*,u32,u32,u32*);
void WIN nd_copy_caller_rect(void*,void*,void*,const u32*,u32,u32,u32*);
__asm__(
 ".text\n.p2align 4\n.globl \"_nd_copy_caller_whole@24\"\n\"_nd_copy_caller_whole@24\":\n"
 "pushl %ebp\nmovl %esp,%ebp\nmovl 28(%ebp),%eax\n"
 "pushl 24(%ebp)\npushl 20(%ebp)\npushl 16(%ebp)\n"
 "leal -4(%esp),%edx\nmovl %edx,(%eax)\nmovl 12(%ebp),%ecx\ncalll *8(%ebp)\n"
 ".globl _nd_copy_caller_whole_return\n_nd_copy_caller_whole_return:\n"
 "movl %ebp,%esp\npopl %ebp\nretl $24\n"
 ".p2align 4\n.globl \"_nd_copy_caller_rect@28\"\n\"_nd_copy_caller_rect@28\":\n"
 "pushl %ebp\nmovl %esp,%ebp\nmovl 32(%ebp),%eax\n"
 "pushl 28(%ebp)\npushl 24(%ebp)\npushl 20(%ebp)\npushl 16(%ebp)\n"
 "leal -4(%esp),%edx\nmovl %edx,(%eax)\nmovl 12(%ebp),%ecx\ncalll *8(%ebp)\n"
 ".globl _nd_copy_caller_rect_return\n_nd_copy_caller_rect_return:\n"
 "movl %ebp,%esp\npopl %ebp\nretl $28\n");
