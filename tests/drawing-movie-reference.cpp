// Unchanged original setup/sample/wrapper with explicit synthetic COM/IAT bindings.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <limits>
#include <vector>
#define STDCALL __attribute__((stdcall))
struct Object {std::uint32_t* table;int refs;};
struct Rect {std::uint32_t left,top,right,bottom;};
static std::uint32_t stream_table[17],media_table[3],video_table[14],sample_table[9],surface_table[6];
static Object stream{stream_table,0},media{media_table,0},video{video_table,0},sample{sample_table,0},source{surface_table,0},target_surface{surface_table,0},target_device{surface_table,0};
static std::vector<int> release_order;
static int failure,updates,blits,escapes,update_error,blit_error,stage_calls[10],release_calls[5];
static Rect source_rect{5,7,645,367},actual_destination{},actual_source{};
static std::uint32_t x_position,y_position;
static int control_value;
static bool placement_valid,arguments_valid;
static std::int32_t status(int stage){
 ++stage_calls[stage];
 arguments_valid &= *reinterpret_cast<std::uint32_t*>(0x5fbf50)==(stage<=4?0x11223344u:x_position);
 arguments_valid &= *reinterpret_cast<std::uint32_t*>(0x5fbf54)==(stage<=4?0x55667788u:y_position);
 arguments_valid &= *reinterpret_cast<std::uint32_t*>(0x5fbf5c)==(stage<=4?0xdeadbeefu:std::uint32_t(control_value));
 return failure==stage?std::int32_t(0x80004005u):0;
}
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static std::uint32_t ptr(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
static std::uint32_t STDCALL addref(Object* self){return ++self->refs;}
static std::uint32_t STDCALL release(Object* self){
 for(unsigned i=0;i<5;++i)if(self==std::array<Object*,5>{&stream,&media,&video,&sample,&source}[i]){++release_calls[i];release_order.push_back(i);}
 require(self->refs>0,"Release without an owned synthetic reference");return --self->refs;
}
static std::int32_t STDCALL create(const void* clsid,void* outer,std::uint32_t context,const void* iid,Object** out){
 arguments_valid &= clsid==reinterpret_cast<void*>(0x5c9230)&&!outer&&context==1&&iid==reinterpret_cast<void*>(0x5c91b0);
 auto result=status(0);*out=result<0?nullptr:&stream;if(result>=0)stream.refs=1;return result;
}
static std::int32_t STDCALL initialize(Object* self,std::uint32_t type,std::uint32_t flags,void* graph){
 arguments_valid &= self==&stream&&!type&&!flags&&!graph;return status(1);
}
static std::int32_t STDCALL addmedia(Object* self,Object* device,const void* purpose,std::uint32_t flags,void* out){
 const int stage=stage_calls[2]?3:2;
 arguments_valid &= self==&stream&&!out&&(stage==2?device==&target_device&&purpose==reinterpret_cast<void*>(0x5c92d0)&&!flags:!device&&purpose==reinterpret_cast<void*>(0x5c92e0)&&flags==1);
 return status(stage);
}
static int STDCALL wide(std::uint32_t cp,std::uint32_t flags,const char* path,int length,std::uint16_t* output,int capacity){
 arguments_valid &= !cp&&!flags&&length==-1&&capacity==260&&std::string(path)=="fixture.avi";
 unsigned i=0;do{output[i]=static_cast<unsigned char>(path[i]);}while(path[i++]&&i<unsigned(capacity));return i;
}
static std::int32_t STDCALL openfile(Object* self,const std::uint16_t* path,std::uint32_t flags){
 arguments_valid &= self==&stream&&!flags;
 const char* expected="fixture.avi";for(unsigned i=0;i<12;++i)arguments_valid &= path[i]==static_cast<unsigned char>(expected[i]);
 return status(4);
}
static std::int32_t STDCALL getmedia(Object* self,const void* purpose,Object** out){
 arguments_valid &= self==&stream&&purpose==reinterpret_cast<void*>(0x5c92d0);
 auto result=status(5);*out=result<0?nullptr:&media;if(result>=0)media.refs=1;return result;
}
static std::int32_t STDCALL query(Object* self,const void* iid,Object** out){
 arguments_valid &= self==&media&&iid==reinterpret_cast<void*>(0x5c9270);
 auto result=status(6);*out=result<0?nullptr:&video;if(result>=0)video.refs=1;return result;
}
static std::int32_t STDCALL createsample(Object* self,void* surface,void* rect,std::uint32_t flags,Object** out){
 arguments_valid &= self==&video&&!surface&&!rect&&!flags;
 auto result=status(7);*out=result<0?nullptr:&sample;if(result>=0)sample.refs=1;return result;
}
static std::int32_t STDCALL getsurface(Object* self,Object** out,Rect* rect){
 arguments_valid &= self==&sample;auto result=status(8);*out=result<0?nullptr:&source;
 if(result>=0){source.refs=1;*rect=source_rect;}return result;
}
static std::int32_t STDCALL state(Object* self,std::uint32_t requested){arguments_valid &= self==&stream&&requested==1;return status(9);}
static std::int16_t STDCALL key(int requested){arguments_valid &= requested==27;return escapes?std::int16_t(0x8000):0;}
static std::int32_t STDCALL update(Object* self,std::uint32_t flags,void* event,void* callback,void* data){
 arguments_valid &= self==&sample&&!flags&&!event&&!callback&&!data;
 require(++updates<=2,"Unbounded synthetic sample loop");return update_error?std::int32_t(0x80004005u):updates==1?0:1;
}
static std::int32_t STDCALL blit(Object* self,Rect* destination,Object* source_object,Rect* source_area,std::uint32_t flags,void* effect){
 arguments_valid &= self==&target_surface&&source_object==&source&&flags==0x01000000&&!effect;
 ++blits;actual_destination=*destination;actual_source=*source_area;
 placement_valid &= actual_destination.left==source_rect.left+x_position&&actual_destination.top==source_rect.top+y_position&&actual_destination.right==source_rect.right+x_position&&actual_destination.bottom==source_rect.bottom+y_position;
 placement_valid &= std::memcmp(&actual_source,&source_rect,sizeof(Rect))==0;
 return blit_error?std::int32_t(0x80004005u):0;
}
static int logger(const char*,...){return 0;}
static void jump(std::uint32_t address,const void* replacement){auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;const auto delta=ptr(replacement)-address-5;std::memcpy(code+1,&delta,4);}
int main(int argc,char** argv)try{
 require(argc==2,"Expected pinned executable");map_image(read(argv[1]));
 const Bytes original(reinterpret_cast<unsigned char*>(0x4696e0),reinterpret_cast<unsigned char*>(0x469a5d));
 stream_table[1]=ptr(reinterpret_cast<void*>(addref));stream_table[2]=ptr(reinterpret_cast<void*>(release));
 stream_table[4]=ptr(reinterpret_cast<void*>(getmedia));stream_table[7]=ptr(reinterpret_cast<void*>(state));
 stream_table[12]=ptr(reinterpret_cast<void*>(initialize));stream_table[15]=ptr(reinterpret_cast<void*>(addmedia));stream_table[16]=ptr(reinterpret_cast<void*>(openfile));
 media_table[0]=ptr(reinterpret_cast<void*>(query));media_table[2]=ptr(reinterpret_cast<void*>(release));
 video_table[2]=ptr(reinterpret_cast<void*>(release));video_table[13]=ptr(reinterpret_cast<void*>(createsample));
 sample_table[2]=ptr(reinterpret_cast<void*>(release));sample_table[6]=ptr(reinterpret_cast<void*>(update));sample_table[8]=ptr(reinterpret_cast<void*>(getsurface));
 surface_table[2]=ptr(reinterpret_cast<void*>(release));surface_table[5]=ptr(reinterpret_cast<void*>(blit));
 global(0x5c5318,ptr(reinterpret_cast<void*>(create)));global(0x5c5088,ptr(reinterpret_cast<void*>(wide)));global(0x5c5204,ptr(reinterpret_cast<void*>(key)));jump(0x59ca38,reinterpret_cast<void*>(logger));
 using Wrapper=unsigned (__attribute__((fastcall)) *)(const char*,Object*,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t);
 unsigned cases=0;
 for(failure=-1;failure<10;++failure)for(unsigned previous:{0u,1u})for(int control:{0,1,-1,127,-128})
 for(escapes=0;escapes<2;++escapes)for(update_error=0;update_error<2;++update_error)for(blit_error=0;blit_error<2;++blit_error)
 for(auto positions:std::array<std::pair<std::uint32_t,std::uint32_t>,2>{{{80,120},{0x7fffffffu,0x80000000u}}}){
  x_position=positions.first;y_position=positions.second;control_value=control;updates=blits=0;release_order.clear();placement_valid=arguments_valid=true;
  std::memset(stage_calls,0,sizeof(stage_calls));std::memset(release_calls,0,sizeof(release_calls));
  stream.refs=media.refs=video.refs=sample.refs=source.refs=0;global(0x5fbf58,previous);global(0x5fbf50,0x11223344u);global(0x5fbf54,0x55667788u);global(0x5fbf5c,0xdeadbeefu);
  const auto result=reinterpret_cast<Wrapper>(0x469a00)("fixture.avi",&target_device,ptr(&target_surface),x_position,y_position,unsigned(control));
  const unsigned expected=failure>=0&&failure<=4?!previous:failure>=5?1:escapes?0:1;
  require(result==expected,"Wrapper skip/failure return policy");
  require(*reinterpret_cast<std::uint32_t*>(0x5fbf50)==x_position&&*reinterpret_cast<std::uint32_t*>(0x5fbf54)==y_position&&*reinterpret_cast<std::int32_t*>(0x5fbf5c)==control,"Wrapper placement/signed control side effects");
  require(!stream.refs&&!media.refs&&!video.refs&&!sample.refs&&!source.refs,"Synthetic COM reference imbalance");
  for(int stage=0;stage<10;++stage)
   require(stage_calls[stage]==(failure<0||stage<=failure?1:0),"Original setup/playback failure short circuit");
  std::vector<int> expected_releases;
  if(failure!=0)expected_releases.push_back(0);
  if(failure<0||failure>=5){
   if(failure<0||failure>=6)expected_releases.push_back(1);
   if(failure<0||failure>=7)expected_releases.push_back(2);
   if(failure<0||failure>=8)expected_releases.push_back(3);
   if(failure<0||failure>=9)expected_releases.push_back(4);
   expected_releases.push_back(0);
  }
  if(release_order!=expected_releases){std::cerr<<"failure "<<failure<<" order";for(auto n:release_order)std::cerr<<' '<<n;std::cerr<<" expected";for(auto n:expected_releases)std::cerr<<' '<<n;std::cerr<<'\n';}
  require(release_order==expected_releases,"Original COM release order differs");
  require(!target_device.refs&&!target_surface.refs,"Original body explicitly retained a borrowed graphics argument");
  const unsigned expected_blits=failure<0&&!escapes&&!update_error&&control==0?1:0;
  require(unsigned(blits)==expected_blits&&placement_valid&&arguments_valid,"Original movie call/placement arguments");
  require(Bytes(reinterpret_cast<unsigned char*>(0x4696e0),reinterpret_cast<unsigned char*>(0x469a5d))==original,"Original movie code changed");
  std::cout<<"{\"failure_stage\":"<<failure<<",\"previous_skip\":"<<previous<<",\"control\":"<<control<<",\"escape\":"<<escapes<<",\"update_error\":"<<update_error<<",\"blit_error\":"<<blit_error<<",\"x\":"<<x_position<<",\"y\":"<<y_position<<",\"result\":"<<result<<",\"updates\":"<<updates<<",\"blits\":"<<blits<<",\"stage_calls\":[";
  for(unsigned i=0;i<10;++i){if(i)std::cout<<',';std::cout<<stage_calls[i];}std::cout<<"],\"releases\":[";
  for(unsigned i=0;i<5;++i){if(i)std::cout<<',';std::cout<<release_calls[i];}std::cout<<"],\"release_order\":[";for(unsigned i=0;i<release_order.size();++i){if(i)std::cout<<',';std::cout<<release_order[i];}std::cout<<"]}\n";++cases;
 }
 require(cases==1760,"Case count");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
