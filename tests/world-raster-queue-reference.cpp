#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "world-raster-reference.hpp"
#include <sstream>
static Bytes fastRead(const std::string& name){std::ifstream f(name,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("Open "+name);const auto n=f.tellg();if(n<0||n>128*1024*1024)throw std::runtime_error("Read bound");Bytes b(std::size_t(n),0);f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),n))throw std::runtime_error("Read "+name);return b;}
int main(int argc,char** argv)try{
 if(argc!=6)throw std::runtime_error("Expected PE, producer stream, input list, output TSV, capture directory");
 map_image(fastRead(argv[1]));const auto stream=fastRead(argv[2]);std::vector<std::size_t> offsets;
 for(std::size_t at=64;at<stream.size();){const auto n=u32(stream,at);if(n<96||at+n>stream.size())throw std::runtime_error("Stream extent");offsets.push_back(at);at+=n;}
 std::ifstream list(argv[3]);std::ofstream result(argv[4]);if(!list||!result)throw std::runtime_error("List/result open");unsigned seq,entry,ax;std::string before,reply;
 unsigned count=0;
 while(list>>seq>>entry>>ax>>before>>reply){
  const auto at=offsets.at(seq-1),size=u32(stream,at);Bytes record(stream.begin()+at,stream.begin()+at+size);const auto originalBefore=fastRead(before),nativeReply=fastRead(std::string(argv[5])+"/"+reply);
  if(originalBefore.size()%2||nativeReply.size()!=originalBefore.size()+64)throw std::runtime_error("Native completion extent");
  std::vector<std::uint16_t> pixels(originalBefore.size()/2);std::memcpy(pixels.data(),originalBefore.data(),originalBefore.size());const auto actualAX=originalRaster(record,entry,pixels)&65535u;
  if(actualAX!=ax||std::memcmp(pixels.data(),nativeReply.data()+64,originalBefore.size()))throw std::runtime_error("Exact original raster mismatch sequence "+std::to_string(seq)+" AX "+std::to_string(actualAX)+" expected "+std::to_string(ax));
  result<<seq<<'\t'<<entry<<'\t'<<ax<<'\t'<<pixels.size()<<"\t1\n";++count;
 }
 if(!list.eof()||!count)throw std::runtime_error("Incomplete request list");
 std::cout<<count<<" exact original raster canvases and AX results match\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
