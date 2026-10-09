#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "world-raster-reference.hpp"
#include <array>
#include <map>
#include <sstream>
static Bytes fastRead(const std::string& name){std::ifstream f(name,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("Open "+name);const auto n=f.tellg();if(n<0||n>128*1024*1024)throw std::runtime_error("Read bound");Bytes b(std::size_t(n),0);f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),n))throw std::runtime_error("Read "+name);return b;}
int main(int argc,char** argv)try{
 if(argc!=6)throw std::runtime_error("Expected PE, producer stream, actual-entry TSV, native FIFO, output TSV");
 map_image(fastRead(argv[1]));const auto stream=fastRead(argv[2]);
 std::map<unsigned,std::pair<unsigned,unsigned>> entries;std::ifstream list(argv[3]);unsigned seq,entry,ax;
 while(list>>seq>>entry>>ax){if(!entries.emplace(seq,std::make_pair(entry,ax)).second||ax>1)throw std::runtime_error("Duplicate/invalid original input");}if(!list.eof()||entries.empty())throw std::runtime_error("Incomplete original input list");
 std::ifstream pipe(argv[4],std::ios::binary);std::ofstream out(argv[5]);if(!pipe||!out)throw std::runtime_error("FIFO/result open");
 unsigned queue=0,count=0,completed=0;std::vector<std::uint16_t> original;
 auto native=[&](const Bytes& record,unsigned type){
  Bytes h(40);if(!pipe.read(reinterpret_cast<char*>(h.data()),h.size())||std::memcmp(h.data(),"MNMBPF01",8)||u32(h,8)!=type||u32(h,12)!=u32(record,4)||u32(h,16)!=queue||u32(h,20)!=u32(record,12)||u32(h,24)!=u32(record,20)||u32(h,28)!=u32(record,24))throw std::runtime_error("Native proof frame identity");
  const auto bytes=u32(h,32);if(bytes!=(type==3?0u:u32(h,24)*u32(h,28)*2u)||bytes>2048*2048*2u)throw std::runtime_error("Native proof frame extent");
  Bytes pixels(bytes);if(bytes&&!pipe.read(reinterpret_cast<char*>(pixels.data()),bytes))throw std::runtime_error("Truncated native proof pixels");
  return std::make_pair(std::move(pixels),u32(h,36));
 };
 for(std::size_t at=64;at<stream.size();){
  const auto size=u32(stream,at);if(size<96||size>stream.size()-at)throw std::runtime_error("Producer extent");Bytes r(stream.begin()+at,stream.begin()+at+size);at+=size;const auto kind=u32(r,8);
  if(kind==11){if(queue)throw std::runtime_error("Nested World queue");queue=u32(r,56);auto [pixels,unused]=native(r,1);if(unused)throw std::runtime_error("Entry AX");original.resize(pixels.size()/2);std::memcpy(original.data(),pixels.data(),pixels.size());}
  else if(queue&&kind==9){
   const auto id=u32(r,4);auto i=entries.find(id);if(i==entries.end())throw std::runtime_error("Unlisted actual raster entry");
   const auto actualAX=originalRaster(r,i->second.first,original)&65535u;auto [pixels,nativeAX]=native(r,2);
   if(actualAX!=i->second.second||nativeAX!=actualAX||pixels.size()!=original.size()*2||std::memcmp(original.data(),pixels.data(),pixels.size()))throw std::runtime_error("Exact original batch intermediate mismatch sequence "+std::to_string(id)+" AX "+std::to_string(actualAX));
   out<<id<<'\t'<<i->second.first<<'\t'<<actualAX<<'\t'<<original.size()<<"\t1\n";++count;
  }else if(queue&&kind==12){auto [pixels,unused]=native(r,3);if(!pixels.empty()||unused)throw std::runtime_error("Return frame");queue=0;++completed;}
  else if(queue&&kind!=4&&kind!=10)throw std::runtime_error("Unadmitted original queue producer");
 }
 if(queue||count!=entries.size()||!completed)throw std::runtime_error("Incomplete batch proof");
 char extra;if(pipe.get(extra))throw std::runtime_error("Extra native proof frame");
 std::cout<<count<<" exact original batch intermediate canvases/AX match in "<<completed<<" queues\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
