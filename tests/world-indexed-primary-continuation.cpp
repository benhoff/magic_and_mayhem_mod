// Execute unchanged original 500477..50049d. Only the existing clipped callee
// and common tail are private fixture endpoints; no installed image changes.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
extern "C" {unsigned primary_saved_sp,primary_stack_delta,primary_fallbacks,primary_bad_args;}
extern "C" unsigned primary_fallback(unsigned frame,unsigned x,unsigned y,unsigned shade,unsigned palette){
 if(frame!=0x11112222||x!=0x12345678||y!=0x87654321||shade!=7||palette!=9)++primary_bad_args;
 ++primary_fallbacks;return 0xabcd0003;
}
extern "C" unsigned primary_continue(unsigned,unsigned);
asm(".text\n.globl primary_continue\nprimary_continue:\n"
 "push %ebx\npush %esi\npush %edi\npush %ebp\nmov %esp,primary_saved_sp\n"
 "mov 20(%esp),%eax\nmov 24(%esp),%edx\n"
 "sub $64,%esp\nmovl $1f,0(%esp)\nmovl $9,20(%esp)\nmovl $7,32(%esp)\n"
 "mov $0x11112222,%edi\nmov $0x12345678,%ebp\nmov $0x87654321,%ebx\n"
 "push $1\npush $2\npush $3\npush $4\npush $5\njmp *%edx\n"
 "1: mov primary_saved_sp,%ecx\nsub $60,%ecx\nsub %esp,%ecx\nmov %ecx,primary_stack_delta\n"
 "mov primary_saved_sp,%esp\npop %ebp\npop %edi\npop %esi\npop %ebx\nret\n");
int main(int argc,char**argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned PE image");map_image(read(argv[1]));
 const auto* site=reinterpret_cast<const unsigned char*>(0x500477);
 const std::array<unsigned char,13> expected{0x83,0xc4,0x14,0x66,0x3d,1,0,0x0f,0x85,0xc4,3,0,0};
 if(std::memcmp(site,expected.data(),expected.size()))throw std::runtime_error("Continuation changed");
 const auto before=std::vector<unsigned char>(site,site+39);
 auto* target=reinterpret_cast<unsigned char*>(0x57de00);
 if(std::memcmp(target,"\x55\x8b\xec\x83\xec\x18",6))throw std::runtime_error("Clipped entry changed");
 target[0]=0xe9;const unsigned delta=unsigned(primary_fallback)-0x57de05;std::memcpy(target+1,&delta,4);
 *reinterpret_cast<unsigned char*>(0x500848)=0xc3;
 unsigned cases=0;
 for(unsigned high:{0u,0x12340000u,0xffff0000u})for(unsigned low=0;low<65536;++low){
  const unsigned input=high|low,old=primary_fallbacks;
  const unsigned result=primary_continue(input,0x500477);
  if(result!=(low==1?0xabcd0003:input)||primary_fallbacks!=old+(low==1)||primary_stack_delta||primary_bad_args)
   throw std::runtime_error("Original AX branch, five arguments or cdecl cleanup differs");
  ++cases;
 }
 if(std::memcmp(site,before.data(),before.size()))throw std::runtime_error("Original continuation mutated");
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"fallbacks\":"<<primary_fallbacks<<",\"all_low16_values\":true,\"high_bits_ignored_for_decision\":true,\"arguments_and_stack\":true}\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
