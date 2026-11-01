// Test-only per-draw output hashes around unmodified mapped original routines.
#include <openssl/sha.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
namespace {
struct OriginalDrawWatch {
 unsigned ordinal,entry,kind,mode,backend;
 const std::vector<std::uint16_t>& pixels;
 bool selected=false;std::string prefix;
 void save(const char* suffix)const{
  const auto name=prefix+suffix;auto* f=std::fopen(name.c_str(),"wbx");
  if(!f)throw std::runtime_error("New selected original checkpoint required");
  const auto n=std::fwrite(pixels.data(),2,pixels.size(),f);const auto closed=std::fclose(f);
  if(n!=pixels.size()||closed)throw std::runtime_error("Selected original checkpoint write");
 }
 OriginalDrawWatch(unsigned o,unsigned e,const std::vector<unsigned char>& r,const std::vector<std::uint16_t>& p):ordinal(o),entry(e),kind(0),mode(0),backend(0),pixels(p){
  auto word=[&](unsigned at){return unsigned(r.at(at))|unsigned(r.at(at+1))<<8|unsigned(r.at(at+2))<<16|unsigned(r.at(at+3))<<24;};
  kind=word(8);mode=word(56);backend=word(60);
  const auto* capture=std::getenv("MNM_DIVERGENCE_CAPTURE_DRAW");const auto* output=std::getenv("MNM_DIVERGENCE_ORIGINAL_PREFIX");
  selected=capture&&output&&std::strtoul(capture,nullptr,10)==ordinal;
  if(selected){prefix=output;save("-before.565");}
 }
 ~OriginalDrawWatch()noexcept(false){
  unsigned char hash[SHA256_DIGEST_LENGTH];SHA256(reinterpret_cast<const unsigned char*>(pixels.data()),pixels.size()*2,hash);
  std::fprintf(stderr,"DRAW %u %08x %u %u %u ",ordinal,entry,kind,mode,backend);
  for(auto byte:hash){std::fprintf(stderr,"%02x",unsigned(byte));}
  std::fputc('\n',stderr);
  if(selected)save("-after.565");
 }
};
}
#define MNM_ACCEPTANCE_DRAW_WATCH(o,e,r,p) OriginalDrawWatch originalDrawWatch(o,e,r,p)
#include "acceptance-world-reference.cpp"
