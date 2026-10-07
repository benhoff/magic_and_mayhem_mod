#include "surface_access.hpp"
#include <fstream>
#include <iostream>
#include <type_traits>
#include <vector>
using mnm::render::SurfaceAccessState;
static_assert(!std::is_copy_constructible_v<SurfaceAccessState>);
static_assert(!std::is_move_constructible_v<SurfaceAccessState>);
static void require(bool ok){if(!ok)throw std::runtime_error("Native access check failed");}
static void word(std::ofstream& f,std::uint32_t v){for(unsigned i=0;i<4;++i)f.put(static_cast<char>(v>>(i*8)));}
static void descriptor(std::ofstream& f,SurfaceAccessState::Descriptor d){d[9]=d[9]==0xabababab?0:d[9]?2:1;for(auto v:d)word(f,v);}
int main(int argc,char** argv){try{
 require(argc==3);std::ifstream input(argv[1]);std::ofstream output(argv[2],std::ios::binary);require(bool(input)&&bool(output));unsigned count=0,steps=0,terminalBusy=0,guards=0;input>>count;require(count==96);
 for(unsigned i=0;i<count;++i){unsigned id,bits,caps,n;input>>id>>bits>>caps>>n;require(bool(input)&&id==i&&n<=64);SurfaceAccessState access(8,6,bits,caps,0x1234,0x5678);std::array<std::uint32_t,48> pixels{};for(unsigned j=0;j<48;++j)pixels[j]=(j*17+3)&(bits==8?255:bits==16?65535:0xffffffffu);
  for(unsigned k=0;k<n;++k){unsigned op;input>>op;require(bool(input)&&op<=5);SurfaceAccessState::Descriptor d;d.fill(0xffffffff);std::uint32_t result=0,out=0xffffffff;
   if(op==0){d.fill(0xabababab);d[0]=108;result=access.lock(d);}
   else if(op==1)result=access.unlock();
   else if(op==2){std::uint32_t token=0xabababab;result=access.acquireDc(token);out=token==0xabababab?0:token?2:1;}
   else result=access.releaseDc(op==3?0x5678:op==4?0:0x8765);
   word(output,result);word(output,out);if(op==0)descriptor(output,d);else for(auto v:d)word(output,v);++steps;
  }
  SurfaceAccessState::Descriptor d;d.fill(0xabababab);d[0]=108;auto final=access.lock(d);word(output,final);descriptor(output,d);
  if(!final){for(auto v:pixels)word(output,v);require(access.unlock()==0&&!access.borrowed());}
  else{require(final==SurfaceAccessState::busy&&access.poisoned()&&access.borrowed());++terminalBusy;}
 }
 unsigned unused;require(!(input>>unused));output.close();require(bool(output));
 // Native context/token/layout guards, separate from captured driver behavior.
 for(unsigned k=0;k<9;++k){bool refused=false;try{SurfaceAccessState bad(k==0?0:k==1?2049:8,k==2?0:6,k==3?24:16,k==4?0:0x840,k==5?0:k==6?0xabababab:0x1234,k==7?0:k==8?0x1234:0x5678);}catch(const std::runtime_error&){refused=true;}require(refused);++guards;}
 SurfaceAccessState debt(8,6,16,0x840,0x1234,0x5678);bool refused=false;
 for(unsigned k=0;k<70&&!refused;++k){std::uint32_t dc=0;try{debt.acquireDc(dc);debt.unlock();debt.releaseDc(dc);debt.unlock();}catch(const std::runtime_error&){refused=true;}}
 require(refused&&debt.mapBalance()>=-64&&debt.poisoned());++guards;
 std::cout<<"{\"cases\":"<<count<<",\"steps\":"<<steps<<",\"terminal_busy_cases\":"<<terminalBusy<<",\"native_guards\":"<<guards<<"}\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
