// Unchanged original canvas release client and terminal wrapper flow.
#define main retry_reference_unused_main
#include "surface-retry-reference.cpp"
#undef main
#include <algorithm>
static U raw(void* p){return reinterpret_cast<U>(p);}
static U& global(U address){return *reinterpret_cast<U*>(address);}
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::array<void*,39> release_table;
static std::array<Interface,4> release_surfaces;
static std::array<U,4> calls;
static U schedule4,schedule8,external_result;
static std::vector<U> order;
static I __attribute__((stdcall)) released(void* p){
 const U id=static_cast<Interface*>(p)->id;check(id>=1&&id<=3,"Release receiver");
 ++calls[id];order.push_back(id);
 return id==3?external_result:calls[id]<=(id==1?schedule4:schedule8)?(id==1?0x80004005:1):0;
}
static I __attribute__((stdcall)) auxiliary(void* p,U zero){
 check(p==&release_surfaces[2]&&!zero,"Auxiliary slot152 receiver/argument");order.push_back(38);return external_result;
}
static std::vector<U> wrapper_receivers;
static U close_result,closed_handle,close_calls;
static void __attribute__((thiscall)) wrapper_release(void* p){wrapper_receivers.push_back(raw(p));}
static I __attribute__((stdcall)) close_handle(U handle){closed_handle=handle;++close_calls;return close_result;}
static void bind(U address,void* target){auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;U delta=raw(target)-address-5;std::memcpy(code+1,&delta,4);}
static std::vector<unsigned char> bytes(U start,U end){auto* p=reinterpret_cast<unsigned char*>(start);return {p,p+end-start};}
int main(int argc,char** argv)try{
 check(argc==2,"Expected pinned PE");mapImage(argv[1]);
 const auto original=bytes(0x58b3e0,0x58b4b8);
 const std::array<std::pair<U,U>,5> ranges{{{0x4bdd10,0x4bdd1a},{0x4bdcb0,0x4bdcba},{0x4a2f00,0x4a2f1f},{0x4a2f70,0x4a2f8f},{0x4a37d0,0x4a37fe}}};
 std::array<std::vector<unsigned char>,5> wrappers;for(unsigned i=0;i<ranges.size();++i)wrappers[i]=bytes(ranges[i].first,ranges[i].second);
 release_table[2]=reinterpret_cast<void*>(&released);release_table[38]=reinterpret_cast<void*>(&auxiliary);
 for(unsigned id=1;id<release_surfaces.size();++id)release_surfaces[id]={release_table.data(),id};
 Object object;std::array<Object,128> others;U release_cases=0;
 for(U present:{0u,1u})for(U aux:{0u,1u})for(U first:{0u,2u,1001u})for(U second:{0u,2u,1001u})
 for(U clipper:{0u,1u})for(U current:{0u,1u})for(U index:{0u,1u,126u,127u})for(U tail:{0u,1u,3u})for(U count:{0u,1u,7u}){
  object.words.fill(0xaabbccdd);object.words[1]=raw(&release_surfaces[1]);object.words[2]=present?raw(&release_surfaces[2]):0;
  object.words[3]=clipper?raw(&release_surfaces[3]):0;object.words[7]=index;const auto before=object.words;
  for(unsigned i=0;i<128;++i){others[i].words.fill(0xdeadbeef);others[i].words[7]=i;global(0x6f6950+i*4)=0;}
  for(unsigned i=0;i<index;++i)global(0x6f6950+i*4)=raw(&others[i]);
  global(0x6f6950+index*4)=raw(&object);const U moved=std::min(tail,127-index);
  for(U i=1;i<=moved;++i)global(0x6f6950+(index+i)*4)=raw(&others[index+i]);
  std::array<U,128> list;for(unsigned i=0;i<128;++i)list[i]=global(0x6f6950+i*4);
  global(0x6f8154)=count;global(0x6f98c0)=current?raw(&object):raw(&others[127]);global(0x6e1f1c)=aux;
  schedule4=first;schedule8=second;external_result=count==0?0:count==1?1:0xffffffff;calls.fill(0);order.clear();
  using Release=void(__attribute__((thiscall))*)(Object*);reinterpret_cast<Release>(0x58b3e0)(&object);
  const U n4=present?std::min(first+1,1000u):0;
  const U n8=present?std::min(second+1,std::max(1u,1000u-n4)):0;
  // n4==1000 still executes one +8 Release, sharing the original EDI cap.
  std::vector<U> expected;if(present){if(aux)expected.push_back(38);expected.insert(expected.end(),n4,1);expected.insert(expected.end(),n8,2);if(clipper)expected.push_back(3);}
  check(order==expected&&calls[1]==n4&&calls[2]==n8&&calls[3]==(present&&clipper?1u:0u),"Original shared Release cap/order");
  auto after=before;if(present){for(U word:{1u,2u,4u,5u,6u,10u})after[word]=0;if(clipper)after[3]=0;after[7]=index+moved;}
  check(object.words==after,"Original release object field writes");
  check(global(0x6f8154)==(present?count-1:count),"Original unsigned global decrement");
  check(global(0x6f98c0)==(present&&current?0:current?raw(&object):raw(&others[127])),"Current canvas retirement");
  if(present){for(U i=0;i<moved;++i){list[index+i]=list[index+i+1];check(others[index+i+1].words[7]==index+i,"Compacted receiver index");}list[index+moved]=0;list[127]=0;}
  for(unsigned i=0;i<128;++i){check(global(0x6f6950+i*4)==list[i],"Original canvas list compaction");const bool shifted=present&&i>index&&i<=index+moved;check(others[i].words[7]==(shifted?i-1:i),"Unrelated list receiver index");for(unsigned j=0;j<16;++j)if(j!=7)check(others[i].words[j]==0xdeadbeef,"Unrelated receiver fields");}
  check(bytes(0x58b3e0,0x58b4b8)==original,"Original release instructions changed");
  std::cout<<"{\"kind\":\"release\",\"present\":"<<present<<",\"aux\":"<<aux<<",\"first\":"<<first<<",\"second\":"<<second
   <<",\"clipper\":"<<clipper<<",\"current\":"<<current<<",\"index\":"<<index<<",\"tail\":"<<tail<<",\"count\":"<<count
   <<",\"calls4\":"<<n4<<",\"calls8\":"<<n8<<",\"calls_clipper\":"<<(present&&clipper?1:0)<<",\"moved\":"<<(present?moved:0)<<"}\n";++release_cases;
 }
 check(release_cases==5184,"Release matrix cardinality");
 // Wrapper instructions remain original; only their external callee is mocked.
 bind(0x58b3e0,reinterpret_cast<void*>(&wrapper_release));global(0x5c50a0)=raw(reinterpret_cast<void*>(&close_handle));
 U wrapper_cases=0;using Callback=void(*)();
 for(U target:{0x4bdd10u,0x4bdcb0u,0x4a2f00u,0x4a2f70u})for(U flags=0;flags<256;++flags){
  *reinterpret_cast<unsigned char*>(0x641a70)=flags;wrapper_receivers.clear();
  reinterpret_cast<Callback>(target)();reinterpret_cast<Callback>(target)();
  const U bit=target==0x4a2f00?1:target==0x4a2f70?2:0;
  const U receiver=target==0x4bdd10?0x6a49e0:target==0x4bdcb0?0x6a4fe0:target==0x4a2f00?0x641fc8:0x642000;
  const unsigned expected=bit?(flags&bit?0:1):2;
  check(wrapper_receivers==std::vector<U>(expected,receiver),"Fixed receiver callback/one-shot bit");
  check(*reinterpret_cast<unsigned char*>(0x641a70)==(flags|bit),"One-shot flag exact preservation");
  std::cout<<"{\"kind\":\"callback\",\"target\":"<<target<<",\"flags\":"<<flags<<",\"calls\":"<<expected<<"}\n";++wrapper_cases;
 }
 for(U handle:{0u,0x12345678u,0xffffffffu})for(U result:{0u,1u,0xffffffffu}){
  global(0x642c58)=handle;close_result=result;closed_handle=close_calls=0;wrapper_receivers.clear();
  reinterpret_cast<Callback>(0x4a37d0)();
  check(closed_handle==handle&&close_calls==(handle?1u:0u)&&global(0x642c58)==0,"Optional CloseHandle result ignored/global cleared");
  check(wrapper_receivers==std::vector<U>({0x642000,0x641fc8}),"Outer cleanup fixed receiver order");
  std::cout<<"{\"kind\":\"outer\",\"handle\":"<<handle<<",\"result\":"<<result<<",\"calls\":2}\n";++wrapper_cases;
 }
 check(wrapper_cases==1033,"Wrapper matrix cardinality");
 for(unsigned i=0;i<ranges.size();++i)check(bytes(ranges[i].first,ranges[i].second)==wrappers[i],"Original wrapper instructions changed");
 return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
