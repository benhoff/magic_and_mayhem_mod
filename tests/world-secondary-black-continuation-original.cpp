// Execute only the unchanged secondary queue continuations. Private test tails
// stop before the original fallback body or the remainder of queue traversal.
#define main secondary_black_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
extern "C" std::uint64_t secondary_black_probe(unsigned,unsigned);
extern "C" std::uint64_t secondary_fallback_probe(unsigned,unsigned,void*);
asm(".text\n.globl secondary_black_probe\nsecondary_black_probe:\n"
    "push %ebx\npush %esi\npush %edi\npush %ebp\n"
    "mov 20(%esp),%eax\nmov 24(%esp),%ecx\npush $1f\n"
    "push $0x11223344\npush $0x55667788\npush $0x99aabbcc\njmp *%ecx\n"
    "1: pop %ebp\npop %edi\npop %esi\npop %ebx\nret\n"
    ".globl secondary_fallback_probe\nsecondary_fallback_probe:\n"
    "push %ebx\npush %esi\npush %edi\npush %ebp\n"
    "mov 20(%esp),%eax\nmov 24(%esp),%ecx\nmov 28(%esp),%esi\npush $2f\n"
    "push $1\npush $-127\npush $0x11223344\npush $0x55667788\npush $0x99aabbcc\njmp *%ecx\n"
    "2: pop %ebp\npop %edi\npop %esi\npop %ebx\nret\n");
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned PE32 image");
 map_image(read(argv[1]));
 const unsigned site=0x500a16,clipped=0x500a32;
 const std::array<unsigned char,16> black{0x83,0xc4,0x0c,0x66,0x48,0x66,0xf7,0xd8,0x1b,0xc0,0x40,0x66,0x85,0xc0,0x74,0xac};
 const std::array<unsigned char,12> fallback{0x83,0xc4,0x14,0x66,0x89,0x46,0x1e,0xe9,0xc7,0x02,0,0};
 auto* b=reinterpret_cast<const unsigned char*>(site);auto* f=reinterpret_cast<const unsigned char*>(clipped);
 std::int32_t delta;
 if(std::memcmp(b,black.data(),black.size())||std::memcmp(f,fallback.data(),fallback.size())||b[-5]!=0xe8||f[-5]!=0xe8)
  throw std::runtime_error("Secondary direct continuation changed");
 std::memcpy(&delta,b-4,4);if(site+delta!=0x595677)throw std::runtime_error("Black direct target changed");
 std::memcpy(&delta,f-4,4);if(clipped+delta!=0x57de00)throw std::runtime_error("Clipped direct target changed");
 if(site+16+std::int8_t(b[15])!=0x5009d2)throw std::runtime_error("Black AX0 destination changed");
 std::memcpy(&delta,f+8,4);if(clipped+12+delta!=0x500d05)throw std::runtime_error("Clipped AX store destination changed");
 const unsigned char zero[]={0x31,0xd2,0xc3},one[]={0xba,1,0,0,0,0xc3},stored[]={0x0f,0xb7,0x56,0x1e,0xc3};
 std::memcpy(reinterpret_cast<void*>(0x5009d2),zero,sizeof(zero));
 std::memcpy(reinterpret_cast<void*>(site+16),one,sizeof(one));
 std::memcpy(reinterpret_cast<void*>(0x500d05),stored,sizeof(stored));
 unsigned black_cases=0,fallback_cases=0;
 for(unsigned high:{0u,0x12340000u,0xffff0000u})for(unsigned low=0;low<65536;++low){
  const unsigned value=high|low,expected=low==1?1:0;
  const auto branch=secondary_black_probe(value,site);
  if(unsigned(branch)!=expected||unsigned(branch>>32)!=expected)throw std::runtime_error("Secondary AX normalization/branch differs");
  ++black_cases;std::array<unsigned char,64> node;node.fill(0x5a);
  const auto result=secondary_fallback_probe(value,clipped,node.data());
  if(unsigned(result)!=value||unsigned(result>>32)!=low)throw std::runtime_error("Secondary AX store/EAX differs");
  for(unsigned i=0;i<node.size();++i)if(i!=0x1e&&i!=0x1f&&node[i]!=0x5a)throw std::runtime_error("Secondary store changed other bytes");
  ++fallback_cases;
 }
 if(std::memcmp(b,black.data(),black.size())||std::memcmp(f,fallback.data(),fallback.size()))throw std::runtime_error("Original selected fragment mutated");
 std::cout<<"{\"success\":true,\"black_cases\":"<<black_cases<<",\"fallback_cases\":"<<fallback_cases<<",\"low16_values\":65536,\"high16_patterns\":3,\"black_cdecl_cleanup_bytes\":12,\"fallback_cdecl_cleanup_bytes\":20,\"fallback_iff_ax_one\":true,\"node_store_offset\":30,\"node_store_bytes\":2,\"EAX_preserved_by_fallback_store\":true,\"selected_fragments_unchanged\":true,\"original_raster_body_executed\":false}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
