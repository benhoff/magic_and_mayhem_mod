// Execute only the unchanged pinned AX-normalization fragments. Private test
// tails stop at the original branch destinations before any fallback body.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
extern "C" std::uint64_t black_continue_probe(unsigned,unsigned);
asm(".text\n.globl black_continue_probe\nblack_continue_probe:\n"
    "push %ebx\npush %esi\npush %edi\npush %ebp\n"
    "mov 20(%esp),%eax\nmov 24(%esp),%ecx\npush $1f\n"
    "push $0x11223344\npush $0x55667788\npush $0x99aabbcc\njmp *%ecx\n"
    "1: pop %ebp\npop %edi\npop %esi\npop %ebx\nret\n");
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned PE32 image");
 map_image(read(argv[1]));
 const std::array<unsigned char,16> prefix{0x83,0xc4,0x0c,0x66,0x48,0x66,0xf7,0xd8,0x1b,0xc0,0x40,0x66,0x85,0xc0,0x0f,0x84};
 const unsigned char skip[]={0x31,0xd2,0xc3},fallback[]={0xba,1,0,0,0,0xc3};
 // Both selected fragments branch to the common original AX store.
 std::memcpy(reinterpret_cast<void*>(0x500848),skip,sizeof(skip));
 unsigned cases=0;
 for(unsigned site:{0x500439u,0x5004d4u}){
  const auto* p=reinterpret_cast<const unsigned char*>(site);
  std::int32_t delta;std::memcpy(&delta,p+16,4);
  if(std::memcmp(p,prefix.data(),prefix.size())||site+20+delta!=0x500848||p[-5]!=0xe8)
   throw std::runtime_error("Black continuation changed");
  std::memcpy(&delta,p-4,4);
  if(site+delta!=0x595677)throw std::runtime_error("Black direct callee changed");
  const std::array<unsigned char,20> before=[&]{std::array<unsigned char,20>x{};std::memcpy(x.data(),p,20);return x;}();
  std::memcpy(reinterpret_cast<void*>(site+20),fallback,sizeof(fallback));
  for(unsigned high:{0u,0x12340000u,0xffff0000u})for(unsigned low=0;low<65536;++low){
   const auto result=black_continue_probe(high|low,site);
   const unsigned expected=low==1?1:0;
   if(unsigned(result)!=expected||unsigned(result>>32)!=expected)
    throw std::runtime_error("Original AX normalization/branch differs");
   ++cases;
  }
  if(std::memcmp(p,before.data(),before.size()))throw std::runtime_error("Original fragment mutated");
 }
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"sites\":2,\"low16_values\":65536,\"high16_patterns\":3,\"cdecl_cleanup_bytes\":12,\"fallback_iff_ax_one\":true,\"selected_fragments_unchanged\":true,\"original_raster_body_executed\":false}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
