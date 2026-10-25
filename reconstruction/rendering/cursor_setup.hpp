#pragma once
#include "cursor_clients.hpp"
#include <string>
namespace mnm::reconstruction::cursor {
// NoCD4a30b0 caller control. Bounded, null-terminated paths/profile outputs and
// valid signed atoi inputs only. CRT/OS/resource calls are explicit ports;
// allocator ownership, allocation failure semantics and live writers are not
// inferred from this caller. Missing descriptor keys preserve their old words.
template<class Memory,class Services> class Setup {
 using I=std::int32_t;
 Memory& m;Services& p;
 I boundedInteger(const char* text,I maximum){
  // Preserve repeated conversion calls; ports may observe/mutate their inputs.
  if(p.integer(text)>0 && p.integer(text)>=maximum)return maximum;
  if(p.integer(text)<=0)return 0;
  return p.integer(text);
 }
public:
 Setup(Memory& memory,Services& services):m(memory),p(services){}
 U run(){
  std::string path=p.currentDirectory();
  if(!path.empty() && path.back()!='\\' && path.back()!='/')path+=p.text(0x5d9058);
  path+=p.text(0x5e0550);
  auto profile=[&](const std::string& section,U key){return p.profile(section,p.text(key),p.text(0x5faf28),path);};
  auto widthText=profile(p.text(0x5da478),0x5e0544);
  const I width=boundedInteger(widthText.text.c_str(),1000);
  auto heightText=profile(p.text(0x5da478),0x5e0534);
  const I height=boundedInteger(heightText.text.c_str(),1000);
  for(U type=0;type<22;++type){
   const auto section=std::string(p.text(0x5e052c))+p.decimal(type);
   const U descriptor=0x642678+type*68;
   for(U axis=0;axis<2;++axis){auto value=profile(section,axis?0x5e0514:0x5e0520);if(value.result)m.set32(descriptor+4*axis,U(boundedInteger(value.text.c_str(),40)));}
   auto frames=profile(section,0x5e050c);
   if(frames.result){
    // One owned mutable buffer; the tokenizer owns only its iteration state.
    U count=0;char* token=p.tokenize(frames.text.data(),p.text(0x5da820));
    while(token && count<12){
     char* equals=p.find(token,'=');if(equals)*equals=0;else p.find(token,0);
     m.set32(descriptor+12+count*4,U(p.integer(token)));++count;
     if(count<12)token=p.tokenize(nullptr,p.text(0x5da820));
    }
    m.set32(descriptor+8,count);
   }
   auto countdown=profile(section,0x5e0504);if(countdown.result)m.set32(descriptor+64,U(boundedInteger(countdown.text.c_str(),1000)));
  }
  std::array<U,27> descriptor{};descriptor[0]=108;descriptor[1]=7;descriptor[2]=descriptor[3]=40;descriptor[26]=0x40;
  p.allocate(0x641fc8,descriptor,0x5e04e4,0xb8);p.fill(0x641fc8,31);
  descriptor={};descriptor[0]=108;descriptor[1]=7;descriptor[2]=U(height);descriptor[3]=U(width);descriptor[26]=0x40;
  p.allocate(0x642000,descriptor,0x5e04e4,0xc2);p.fill(0x642000,31);
  p.key(0x642000,0,0,255);p.bitmap(0x642000,0x5e04d0);m.set32(0x642028,0xffffffffu);
  m.set32(0x641a8c,U(width/40));m.set32(0x642c50,U(height/40));m.set32(0x642c58,p.createMutex());
  return 1;
 }
};
}
