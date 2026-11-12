// Execute the unchanged foreground body with owned endpoints. The two copy
// requests are observations; fake COM success does not establish valid pixels.
#define CURSOR_REFERENCE_LIBRARY
#include "cursor-ownership-reference.cpp"
int main(int argc,char** argv)try{
 check(argc==2,"Expected pinned PE");mapImage(argv[1]);
 vtable.fill(nullptr);vtable[2]=reinterpret_cast<void*>(&ownedRef);
 vtable[5]=reinterpret_cast<void*>(&ownedBlt);vtable[7]=reinterpret_cast<void*>(&ownedFast);
 vtable[13]=reinterpret_cast<void*>(&ownedStatus);
 for(U i=0;i<objects.size();++i)objects[i]={vtable.data(),i+1};
 for(auto [at,fn]:std::initializer_list<std::pair<U,void*>>{
  {0x5c5140,reinterpret_cast<void*>(&ownedWait)},{0x5c513c,reinterpret_cast<void*>(&ownedRelease)},
  {0x5c5210,reinterpret_cast<void*>(&ownedCursor)},{0x5c5298,reinterpret_cast<void*>(&ownedTransform)},
  {0x5c5294,reinterpret_cast<void*>(&ownedIntersect)}})mem(at)=reinterpret_cast<U>(fn);
 auto* code=reinterpret_cast<unsigned char*>(0x4a45b0);
 std::vector<unsigned char> before(code,code+849);
 U calls[2]={};
 for(U high=0;high<2;++high){
  reset();px=640;py=512;mem(0x6de6d5)=high;mem(0x6e1ee4)=0;mem(0x6f68d8)=1;
  mem(0x642678)=0;mem(0x64267c)=0;mem(0x642680)=1;
  for(U at:{0x6a49e0u,0x6a4fe0u}){mem(at+20)=640;mem(at+24)=480;}
  trace.clear();reinterpret_cast<void(*)()>(0x4a45b0)();
  for(const auto& row:trace)if(row[0]==11){
   if(calls[high]==0)check(row[1]==1&&row[2]==3&&row[3]==0&&row[4]==0&&
    row[5]==640&&row[6]==512&&row[7]==680&&row[8]==552,"Different save-under request");
   else check(row[1]==3&&row[2]==2&&row[3]==640&&row[4]==512&&
    row[5]==0&&row[6]==0&&row[7]==40&&row[8]==40,"Different keyed cursor request");
   ++calls[high];
  }
 }
 check(calls[0]==0&&calls[1]==2,"Resolution mismatch did not reproduce exact failed pair");
 check(std::equal(before.begin(),before.end(),code),"Original foreground code changed");
 std::cout<<"{\"success\":true,\"low_mode_copy_calls\":0,\"high_mode_copy_calls\":2,"
  "\"source_size\":[640,480],\"save_rectangle\":[640,512,680,552],"
  "\"sprite_destination\":[640,512],\"original_instructions_unchanged\":true}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
