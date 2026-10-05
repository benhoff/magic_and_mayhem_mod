#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_lighting.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
using namespace mnm::reconstruction;
static EffectLightingTable::Fields fields;
static unsigned requests=0,loads=0;
#ifdef ORIGINAL_REFERENCE
static std::string directory;
static unsigned __attribute__((stdcall)) currentDirectory(unsigned capacity,char* output){
 if(capacity!=256 || directory.size()>=capacity)throw std::runtime_error("Directory contract");
 std::memcpy(output,directory.c_str(),directory.size()+1);return directory.size();
}
static unsigned __attribute__((stdcall)) profile(const char* section,const char* key,const char* fallback,char* output,unsigned capacity,const char* path){
 static const char* keys[]={"LightSourceDiameter","LightSourceAffected","ClippedToHeight"};
 const unsigned i=requests/3,k=requests%3;
 auto expectedPath=directory;if(!expectedPath.empty() && expectedPath.back()!='/' && expectedPath.back()!='\\')expectedPath+='\\';expectedPath+="cfg\\effects.cfg";
 if(i>=89 || section!=std::string("FXA_")+std::to_string(i) || std::strcmp(key,keys[k]) || *fallback || capacity!=256 || expectedPath!=path)
  throw std::runtime_error("Profile request contract");
 const auto& value=fields[i][k];std::memcpy(output,value.c_str(),value.size()+1);++requests;return value.size();
}
#endif
static unsigned word(std::istream& input){unsigned n=0;input.read(reinterpret_cast<char*>(&n),4);if(!input)throw std::runtime_error("Input extent");return n;}
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=4)throw std::runtime_error("Expected PE fixtures output");map_image(read(argv[1]));
 const unsigned char expected[]={0x51,0x53,0x55,0x56,0x57,0x83,0xc1,0x0c};
 if(std::memcmp(reinterpret_cast<void*>(0x49c2e0),expected,sizeof(expected)))throw std::runtime_error("Loader entry mismatch");
 global(0x5c50bc,reinterpret_cast<std::uintptr_t>(currentDirectory));global(0x5c507c,reinterpret_cast<std::uintptr_t>(profile));
 using Reload=unsigned(__attribute__((thiscall)) *)(void*);
 using Select=unsigned(__attribute__((thiscall)) *)(void*,unsigned);
 const unsigned char selectBytes[]={0x8b,0x44,0x24,0x04,0x8d,0x44,0x40,0x03};
 if(std::memcmp(reinterpret_cast<void*>(0x49cf60),selectBytes,sizeof(selectBytes)))throw std::runtime_error("Selector entry mismatch");
 const int start=2;
#else
 if(argc!=3)throw std::runtime_error("Expected fixtures output");const int start=1;
#endif
 std::ifstream input(argv[start],std::ios::binary);std::ofstream output(argv[start+1],std::ios::binary);
 const unsigned count=word(input);if(count>1024)throw std::runtime_error("Fixture count");
 for(unsigned seed=0;seed<2;++seed){
  EffectLightingTable table;
  for(auto& e:table.entries())e={seed?37u:0u,seed?7u:0u,seed?9u:0u};
#ifdef ORIGINAL_REFERENCE
  Bytes storage(16+12+89*12+16,0xa5);auto* receiver=storage.data()+16;
  std::memcpy(receiver+12,table.entries().data(),89*12);
#endif
  input.clear();input.seekg(4);
  for(unsigned fixture=0;fixture<count;++fixture){
   for(auto& row:fields)for(auto& value:row){const unsigned n=word(input);if(n>255)throw std::runtime_error("String extent");value.resize(n);input.read(value.data(),n);if(!input)throw std::runtime_error("String extent");}
   table.reload(fields);
#ifdef ORIGINAL_REFERENCE
   directory=fixture%4==0?"":fixture%4==1?"C:\\game":fixture%4==2?"C:\\game\\":"C:/game/";
   requests=0;if(reinterpret_cast<Reload>(0x49c2e0)(receiver)!=1 || requests!=267)throw std::runtime_error("Loader return/request count");
   if(std::memcmp(receiver+12,table.entries().data(),89*12))throw std::runtime_error("Table differs fixture "+std::to_string(fixture));
   for(unsigned i=0;i<89;++i)if(reinterpret_cast<Select>(0x49cf60)(receiver,i)!=table.lightIndex(i))throw std::runtime_error("Selector differs");
   for(unsigned i=0;i<storage.size();++i)if((i<28 || i>=28+89*12) && storage[i]!=0xa5)throw std::runtime_error("Receiver guard changed");
#else
   requests=267;
#endif
   for(const auto& e:table.entries())for(auto n:{e.diameter,e.affected,e.clippedToHeight})output.write(reinterpret_cast<const char*>(&n),4);
   ++loads;
  }
 }
 if(!output)throw std::runtime_error("Output failure");
 std::cout<<"{\"table_loads\":"<<loads<<",\"profile_reads\":"<<loads*267<<",\"compared_dwords\":"<<loads*267<<",\"selectors\":"<<loads*89<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
