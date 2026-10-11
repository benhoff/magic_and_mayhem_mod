// Unchanged original Lock client with explicit scripted external boundaries.
#define main retry_reference_unused_main
#include "surface-retry-reference.cpp"
#undef main
#include <algorithm>
static U lock_results[2],lock_pitches[2],lock_pixels[2],lock_at,restore_result;
static std::array<void*,33> lock_table;
static std::array<Interface,8> lock_surfaces;
static std::vector<U> lock_tags;
static Object lock_object;
static U lock_raw(void* p){return reinterpret_cast<U>(p);}
static U lock_global(U at){return *reinterpret_cast<U*>(at);}
static void lock_set(U at,U value){*reinterpret_cast<U*>(at)=value;}
static void lock_check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void lock_note(U tag,U id,U result=0,U a=0,U b=0){lock_tags.push_back(tag);event(tag,id,result,0,a,b);}
static I __attribute__((stdcall)) lock_api(void* receiver,void* rect,U* description,U flags,void* wait){
 lock_check(lock_at<2&&receiver==&lock_surfaces[1]&&!rect&&!wait&&flags==1&&description==reinterpret_cast<U*>(0x6f68e0),"Lock receiver/arguments/budget");
 lock_check(description[0]==108,"Original DDS size");
 if(!lock_at)for(unsigned i=1;i<27;++i)lock_check(description[i]==0,"Original cleared descriptor");
 description[4]=lock_pitches[lock_at];description[9]=lock_pixels[lock_at];
 U result=lock_results[lock_at++];lock_note(25,1,result,description[4],description[9]);return result;
}
static I __attribute__((stdcall)) lock_restore(void* p){lock_check(p==&lock_surfaces[1],"Restore receiver");lock_note(27,1,restore_result);return restore_result;}
static I __attribute__((stdcall)) lock_key(void* p,U flags,U* range){
 lock_check(p==&lock_surfaces[1]&&flags==8&&range[0]==0x5678&&range[1]==0x5678,"Restore key args/low16");lock_note(29,1,keysResult,range[0],range[1]);return keysResult;
}
static void __attribute__((thiscall)) lock_reload(Interface* p,U argument){lock_check(p->id==1&&argument==0,"Reload callback args");lock_note(100,1);}
static void __attribute__((thiscall)) lock_cursor(void*){lock_note(101,0);}
static void __attribute__((thiscall)) lock_report(void* p,U result){lock_check(lock_raw(p)==0x6f8158,"Report receiver");lock_note(102,0,result);}
static I __attribute__((stdcall)) lock_release(void* p){auto* item=static_cast<Interface*>(p);lock_note(2,item->id);return 0;}
static I __attribute__((stdcall)) lock_display(void* p){lock_check(p==&lock_surfaces[4],"RestoreDisplay receiver");lock_note(19,4);return 0x80004005;}
static I __attribute__((stdcall)) lock_unlock(void* p,void* pixels){lock_check(p==&lock_surfaces[7],"Auxiliary release receiver");lock_note(32,7,0x80004005,lock_raw(pixels));return 0x80004005;}
static I __attribute__((stdcall)) lock_destroy(void* window){lock_check(lock_raw(window)==0x10293847,"Window identity");lock_note(103,0);return 0;}
static void lock_bind(U address,void* target){auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;U delta=lock_raw(target)-address-5;std::memcpy(code+1,&delta,4);}
int main(int argc,char** argv)try{
 lock_check(argc==2,"Expected pinned original PE");mapImage(argv[1]);
 const auto* start=reinterpret_cast<const unsigned char*>(0x58b660);const std::vector<unsigned char> original(start,start+(0x58b815-0x58b660));
 lock_bind(0x484c00,reinterpret_cast<void*>(&lock_report));lock_bind(0x4a2f90,reinterpret_cast<void*>(&lock_cursor));
 lock_set(0x5c5208,lock_raw(reinterpret_cast<void*>(&lock_destroy)));
 lock_table[2]=reinterpret_cast<void*>(&lock_release);lock_table[19]=reinterpret_cast<void*>(&lock_display);
 lock_table[25]=reinterpret_cast<void*>(&lock_api);lock_table[27]=reinterpret_cast<void*>(&lock_restore);lock_table[29]=reinterpret_cast<void*>(&lock_key);lock_table[32]=reinterpret_cast<void*>(&lock_unlock);
 for(unsigned i=1;i<lock_surfaces.size();++i)lock_surfaces[i]={lock_table.data(),i};
 std::array<void*,1> callbacks{reinterpret_cast<void*>(&lock_reload)};Interface reloader{callbacks.data(),1};std::array<U,32> pixels{};
 unsigned cases=0;
 for(unsigned present:{0u,1u})for(U initial:{0u,1u,0x887601c2u,0x80004005u})for(U restored:{0u,1u,0x80004005u})
 for(U retry:{0u,1u,0x887601c2u,0x80004005u})for(unsigned pointer:{0u,1u})for(unsigned colour:{0u,1u})
 for(unsigned reload_mode:{0u,1u,2u})for(unsigned teardown:{0u,1u,2u})for(unsigned release_early:{0u,1u})for(U key_result:{0u,0x80004005u}){
  lock_object={};lock_object.words[1]=lock_raw(&lock_surfaces[7]);lock_object.words[2]=present?lock_raw(&lock_surfaces[1]):0;
  lock_object.words[6]=0xc0ffee42;lock_object.words[10]=reload_mode==2?0xffffffff:reload_mode==1?lock_raw(&reloader):0;lock_object.words[11]=0x12345678;
  const auto object_before=lock_object.words;
  lock_results[0]=initial;lock_results[1]=retry;lock_pitches[0]=1201;lock_pitches[1]=2049;
  lock_pixels[0]=pointer?lock_raw(pixels.data()):0;lock_pixels[1]=pointer?lock_raw(pixels.data()+8):0;
  lock_at=0;restore_result=restored;keysResult=key_result;events.clear();lock_tags.clear();
  lock_set(0x6f8150,0xdeadface);lock_set(0x6de6dd,colour);lock_set(0x6e1f1c,release_early);lock_set(0x6f6950,0);
  lock_set(0x6f68d8,teardown==2);lock_set(0x656614,0x10293847);
  for(auto pair:std::array<std::pair<U,unsigned>,5>{{{0x6f98c4,3},{0x6f98c8,4},{0x6f98cc,5},{0x6f98d0,6},{0x6f98c0,1}}})
   lock_set(pair.first,teardown?lock_raw(&lock_surfaces[pair.second]):0);
  using Lock=void*(__attribute__((thiscall))*)(Object*);void* returned=reinterpret_cast<Lock>(0x58b660)(&lock_object);
  std::vector<U> expected;const bool lost=present&&initial==0x887601c2,restored_ok=lost&&restored==0;
  const U saved=present?(lost?restored:initial):0xdeadface;const bool failed=present&&saved!=0;
  if(present){expected={25,102};if(lost){expected.push_back(27);if(restored_ok){if(colour)expected.push_back(29);if(reload_mode)expected.push_back(reload_mode==1?100:101);}expected.push_back(25);}
   if(failed){if(teardown){if(teardown==1)expected.push_back(2);else expected.push_back(19);expected.insert(expected.end(),{2,2,2});}expected.push_back(103);}if(release_early)expected.push_back(32);}
  lock_check(lock_tags==expected,"Original Lock recovery/teardown order");lock_check(lock_at==(present?(lost?2u:1u):0u),"Lock retry count");
  const U expected_pixels=present?lock_pixels[lost?1:0]:0,expected_pitch=present?lock_pitches[lost?1:0]:0xc0ffee42;
  lock_check(lock_raw(returned)==expected_pixels&&lock_object.words[6]==expected_pitch,"Original returned pointer/final pitch");
  auto object_expected=object_before;object_expected[6]=expected_pitch;lock_check(lock_object.words==object_expected,"Untouched object fields");
  lock_check(lock_global(0x6f8150)==saved,"Stored result is Restore not retried Lock result");
  if(failed&&teardown)for(U address:{0x6f98c4u,0x6f98c8u,0x6f98ccu,0x6f98c0u,0x6f68d8u})lock_check(lock_global(address)==0,"Failed Lock teardown reset");
  lock_check(lock_global(0x6f98d0)==(teardown?(failed&&teardown==1?0:lock_raw(&lock_surfaces[6])):0),"Conditional auxiliary interface retirement");
  if(present&&release_early)lock_check(events.back().a==expected_pixels,"Auxiliary release borrows returned descriptor pointer");
  lock_check(std::vector<unsigned char>(start,start+original.size())==original,"Original Lock text changed");
  std::cout<<"{\"present\":"<<present<<",\"initial\":"<<initial<<",\"restored\":"<<restored<<",\"retry\":"<<retry
   <<",\"pointer\":"<<pointer<<",\"colour\":"<<colour<<",\"reload\":"<<reload_mode<<",\"teardown\":"<<teardown
   <<",\"release_early\":"<<release_early<<",\"key_result\":"<<key_result<<",\"stored_result\":"<<saved
   <<",\"returned_nonnull\":"<<(returned?"true":"false")<<",\"pitch\":"<<expected_pitch<<",\"tags\":[";
  for(unsigned i=0;i<lock_tags.size();++i){if(i)std::cout<<',';std::cout<<lock_tags[i];}std::cout<<"]}\n";++cases;
 }
 lock_check(cases==13824,"Lock matrix size");return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
