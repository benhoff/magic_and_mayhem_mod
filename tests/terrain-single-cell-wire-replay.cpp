// Actual V1 decoder used by the persistent worker, from source-only requests.
// Original oracle bytes are used solely for comparison, never as decoder input.
#define main terrainSingleCellReplay
#include "../tools/terrain-light-replay.cpp"
#undef main
#include <algorithm>
#include <array>
#include <sstream>
#include <sys/mman.h>
#include <unistd.h>
struct RamFiles {
 int source=memfd_create("size-one-input",MFD_CLOEXEC),reply=memfd_create("size-one-reply",MFD_CLOEXEC);
 ~RamFiles(){if(source>=0)close(source);if(reply>=0)close(reply);}
};
static Bytes evaluate(const Bytes& source){
 static RamFiles files;
 if(files.source<0 || files.reply<0 || ftruncate(files.source,0) || ftruncate(files.reply,0))throw std::runtime_error("Fixture RAM files");
 const auto in=std::string("/proc/self/fd/")+std::to_string(files.source),out=std::string("/proc/self/fd/")+std::to_string(files.reply);
 {std::ofstream f(in,std::ios::binary);f.write(reinterpret_cast<const char*>(source.data()),source.size());if(!f)throw std::runtime_error("Fixture input write");}
 std::string label="size-one-replay",input=in,output=out;char* args[]={label.data(),input.data(),output.data()};
 std::ostringstream captured;auto* previous=std::cout.rdbuf(captured.rdbuf());
 const int result=terrainSingleCellReplay(3,args);std::cout.rdbuf(previous);
 if(result)throw std::runtime_error("Decoder refused input");
 std::ifstream f(out,std::ios::binary);return Bytes(std::istreambuf_iterator<char>(f),{});
}
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected oracle bundle and native output");
 std::ifstream input(argv[1],std::ios::binary);std::ofstream output(argv[2],std::ios::binary);
 char magic[8];input.read(magic,8);U cases=0;input.read(reinterpret_cast<char*>(&cases),4);
 if(!input || std::memcmp(magic,"MNMSZ101",8) || cases!=8192)throw std::runtime_error("Oracle bundle identity");
 std::array<bool,256> values{};std::uint64_t compared=0;Bytes first;
 for(U i=0;i<cases;++i){
  U n=0,m=0;input.read(reinterpret_cast<char*>(&n),4);input.read(reinterpret_cast<char*>(&m),4);
  if(!input || n<300 || n>MNM_LIGHT_V1_MAX_BYTES || m<80 || m>MNM_LIGHT_V1_MAX_BYTES)throw std::runtime_error("Oracle bundle bounds");
  Bytes source(n),expected(m);input.read(reinterpret_cast<char*>(source.data()),n);input.read(reinterpret_cast<char*>(expected.data()),m);
  if(!input)throw std::runtime_error("Oracle bundle truncated");
  U word;std::memcpy(&word,source.data()+56,4);const auto inherited=std::int32_t(word);
  if(inherited< -128 || inherited>127)throw std::runtime_error("Oracle byte domain");values[unsigned(inherited+128)]=true;
  const auto actual=evaluate(source);
  if(actual!=expected)throw std::runtime_error("Captured size-one original/native mismatch at request "+std::to_string(i));
  output.write(reinterpret_cast<const char*>(actual.data()),actual.size());compared+=actual.size();
  if(first.empty())first=source;
 }
 if(input.peek()!=std::char_traits<char>::eof() || !std::all_of(values.begin(),values.end(),[](bool v){return v;}))throw std::runtime_error("Oracle bundle coverage/trailer");
 // Noncanonical signed values cannot silently truncate to a byte. Recompute
 // checksums so each refusal reaches the actual configuration admission.
 U refusals=0;
 for(U value:{U(-129),128u,255u,256u,0x80000000u,0x7fffffffu}){
  auto bad=first;std::memcpy(bad.data()+56,&value,4);U check=mnm_light_checksum(bad.data(),U(bad.size()));std::memcpy(bad.data()+120,&check,4);
  bool refused=false;try{evaluate(bad);}catch(const std::runtime_error&){refused=true;}
  if(!refused)throw std::runtime_error("Noncanonical size-one byte admitted");++refusals;
 }
 if(!output)throw std::runtime_error("Native result stream failed");
 std::cout<<"{\"inherited_bytes\":256,\"requests\":"<<cases<<",\"reply_bytes\":"<<compared<<",\"refusals\":"<<refusals<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
