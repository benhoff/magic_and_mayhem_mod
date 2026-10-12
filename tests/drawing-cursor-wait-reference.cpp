// Unchanged original timer cursor entry: bounded type/wait/early-exit gates.
#define main retry_reference_unused_main
#include "surface-retry-reference.cpp"
#undef main
#include <algorithm>
static U& cursor_word(U address){return *reinterpret_cast<U*>(address);}
static U wait_result,release_result,wait_calls,release_calls;
static U __attribute__((stdcall)) cursor_wait(U handle,U delay){
 if(handle!=0x12345678||delay!=10)throw std::runtime_error("Timer outer wait arguments");
 ++wait_calls;return wait_result;
}
static U __attribute__((stdcall)) cursor_release(U handle){
 if(handle!=0x12345678)throw std::runtime_error("Timer outer release arguments");
 ++release_calls;return release_result;
}
static void cursor_check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
int main(int argc,char** argv)try{
 cursor_check(argc==2,"Expected pinned original");mapImage(argv[1]);
 auto* p=reinterpret_cast<unsigned char*>(0x4a4910);const std::vector<unsigned char> original(p,p+0x4d2);
 cursor_word(0x5c5140)=reinterpret_cast<U>(&cursor_wait);cursor_word(0x5c513c)=reinterpret_cast<U>(&cursor_release);
 U cases=0,waits=0,releases=0;
 for(U type:{0xffffffffu,0u,21u,22u,23u})for(U screen:{0u,1u})for(U result:{0u,0x80u,0x102u,0xffffffffu})
 for(U countdown:{0u,1u,2u})for(U returned:{0u,1u}){
  cursor_word(0x5e04c8)=type;cursor_word(0x6e1ec0)=0x12345678;cursor_word(0x6c5038)=screen;
  cursor_word(0x642c60)=0;cursor_word(0x641fc0)=countdown;wait_result=result;release_result=returned;wait_calls=release_calls=0;
  reinterpret_cast<void(*)()>(0x4a4910)();
  const bool admitted=type<=21;const bool owns=admitted&&result==0;
  cursor_check(wait_calls==(admitted?1u:0u)&&release_calls==(owns?1u:0u),"Original outer result/type gate");
  cursor_check(cursor_word(0x641fc0)==(owns&&screen&&countdown?countdown-1:countdown),"Original early countdown write");
  cursor_check(cursor_word(0x6e1ec0)==0x12345678&&cursor_word(0x642c60)==0&&cursor_word(0x6c5038)==screen,"Unrelated timer gate globals");
  cursor_check(std::equal(original.begin(),original.end(),p),"Original timer instructions changed");
  ++cases;waits+=wait_calls;releases+=release_calls;
 }
 cursor_check(cases==240&&waits==96&&releases==24,"Complete outer-gate matrix");
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"wait_calls\":"<<waits<<",\"release_calls\":"<<releases<<"}\n";
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
